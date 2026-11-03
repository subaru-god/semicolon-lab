#!/usr/bin/env bash
set -eu

project_dir="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
voice_url="https://raw.githubusercontent.com/r9y9/pyopenjtalk/master/pyopenjtalk/htsvoice/mei_normal.htsvoice"
voice_license_url="https://raw.githubusercontent.com/r9y9/pyopenjtalk/master/pyopenjtalk/htsvoice/LICENSE_mei_normal.htsvoice"

mkdir -p "$project_dir/runtime/bin" "$project_dir/runtime/dic" "$project_dir/runtime/voice"
curl -fsSL "$voice_url" -o "$project_dir/runtime/voice/mei_normal.htsvoice"
curl -fsSL "$voice_license_url" -o "$project_dir/runtime/voice/LICENSE_mei_normal.htsvoice"

if [[ "$(uname -s)" == "Linux" ]]; then
    work_dir="${TMPDIR:-/tmp}/semicolon-lab-open-jtalk"
    rm -rf "$work_dir"
    mkdir -p "$work_dir"
    cd "$work_dir"

    apt-get download open-jtalk open-jtalk-mecab-naist-jdic libhtsengine1
    package_root="$work_dir/root"
    mkdir -p "$package_root"
    for package_deb in ./*.deb; do
        dpkg-deb -x "$package_deb" "$package_root"
    done
    cp -f "$package_root/usr/bin/open_jtalk" "$project_dir/runtime/bin/open_jtalk"
    rm -rf "$project_dir/runtime/dic"
    mkdir -p "$project_dir/runtime/dic"
    cp -a "$package_root/var/lib/mecab/dic/open-jtalk/naist-jdic/"* \
        "$project_dir/runtime/dic/"
    cp -a "$package_root/usr/lib/x86_64-linux-gnu/libHTS"* \
        "$project_dir/runtime/usr/lib/x86_64-linux-gnu/"
    chmod +x "$project_dir/runtime/bin/open_jtalk"
    echo "Linux用OpenJTalk、NAIST辞書、mei_normalを配置しました。"
else
    cat >&2 <<'EOF'
Linux以外ではOpenJTalk本体とNAIST辞書を手動で配置してください:
  runtime/bin/open_jtalk.exe
  runtime/dic/
  runtime/voice/mei_normal.htsvoice
EOF
fi

if [[ ! -f "$project_dir/runtime/voice/mei_normal.htsvoice" ]]; then
    echo "mei_normal.htsvoiceを配置できませんでした。" >&2
    exit 1
fi
