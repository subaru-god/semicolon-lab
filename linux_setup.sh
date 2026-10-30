#!/usr/bin/env bash
set -eu

project_dir="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
download_dir="${HOME}/semicolon-lab-download"
packages=(espeak-ng mpg123)

cd "$project_dir"
rm -rf runtime
mkdir -p runtime/bin runtime/share
mkdir -p "$download_dir"
cd "$download_dir"

all_packages_available=true
for package in "${packages[@]}"; do
    if ! apt-cache show "$package" >/dev/null 2>&1; then
        all_packages_available=false
        break
    fi
done

if [[ "$all_packages_available" != true ]]; then
    echo "APTのパッケージ一覧に必要なパッケージがありません。更新します..." >&2
    if command -v sudo >/dev/null 2>&1; then
        sudo apt-get update
    else
        apt-get update
    fi
fi

missing_packages=()
for package in "${packages[@]}"; do
    if ! apt-cache show "$package" >/dev/null 2>&1; then
        missing_packages+=("$package")
    fi
done
if ((${#missing_packages[@]} > 0)); then
    printf 'パッケージを取得できません: %s\n' "${missing_packages[*]}" >&2
    echo "Ubuntu/Debianのリポジトリ設定を確認してください。" >&2
    exit 1
fi

dependency_packages=()
while read -r package; do
    [[ -n "$package" ]] && dependency_packages+=("$package")
done < <(
    apt-cache depends --recurse --no-suggests --no-recommends \
        --no-conflicts --no-breaks --no-replaces --no-enhances "${packages[@]}" |
        awk '/^[[:space:]]*(PreDepends|Depends):/ {print $2}' |
        sed 's/:.*//' | grep -v '^<' | sort -u
)

apt download "${packages[@]}" "${dependency_packages[@]}"
for package_deb in ./*.deb; do
    dpkg-deb -x "$package_deb" "$project_dir/runtime"
done

cd "$project_dir"
if [[ -f runtime/usr/bin/espeak-ng ]]; then
    cp -f runtime/usr/bin/espeak-ng runtime/bin/espeak-ng
    chmod +x runtime/bin/espeak-ng
fi
if [[ -f runtime/usr/bin/mpg123 ]]; then
    cp -f runtime/usr/bin/mpg123 runtime/bin/mpg123
elif [[ -f runtime/usr/bin/mpg123.bin ]]; then
    # Ubuntuのmpg123パッケージはupdate-alternatives用に本体をmpg123.binで提供する。
    cp -f runtime/usr/bin/mpg123.bin runtime/bin/mpg123
    chmod +x runtime/bin/mpg123
fi
for executable in espeak-ng mpg123; do
    if [[ ! -x "runtime/bin/$executable" ]]; then
        echo "$executableをruntime/binへ配置できませんでした。" >&2
        exit 1
    fi
done
if [[ ! -d runtime/usr/share/espeak-ng-data &&
      -d runtime/usr/lib/x86_64-linux-gnu/espeak-ng-data ]]; then
    mkdir -p runtime/usr/share
    cp -a runtime/usr/lib/x86_64-linux-gnu/espeak-ng-data runtime/usr/share/
fi
if [[ ! -d runtime/usr/share/espeak-ng-data ]]; then
    echo "espeak-ngの音声データをruntime/へ展開できませんでした。" >&2
    exit 1
fi
echo "espeak-ngとmpg123をruntime/へ展開しました。"