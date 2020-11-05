#!/bin/bash -ex

# "apex" type build targets to build.
APEX_TARGETS=(
  com.android.apex.test.bar
  com.android.apex.test.foo
)

# "genrule" type build targets to build, and directory they are built from.
GENRULE_TARGETS=(
  system/apex/tests/testdata/sharedlibs/build/com.android.apex.test.bar:com.android.apex.test.bar_stripped
  system/apex/tests/testdata/sharedlibs/build/com.android.apex.test.foo:com.android.apex.test.foo_stripped
  system/apex/tests/testdata/sharedlibs/build/com.android.apex.test.sharedlibs:com.android.apex.test.sharedlibs_generated
)

if [ ! -e "build/make/core/Makefile" ]; then
    echo "$0 must be run from the top of the tree"
    exit 1
fi

OUT_DIR=$(source build/envsetup.sh > /dev/null; TARGET_PRODUCT= get_build_var OUT_DIR)
DIST_DIR=$(source build/envsetup.sh > /dev/null; TARGET_PRODUCT= get_build_var DIST_DIR)
TMPDIR=$(source build/envsetup.sh > /dev/null; TARGET_PRODUCT= get_build_var TMPDIR)

genrules=()
for t in "${GENRULE_TARGETS[@]}"; do
    IFS=: read -a ar <<< "${t}"
    genrules+=( ${ar[1]} )
done

build/soong/soong_ui.bash \
    --make-mode \
    TARGET_PRODUCT=aosp_arm64 \
    "${APEX_TARGETS[@]}" \
    "${genrules[@]}"

for t in "${APEX_TARGETS[@]}"; do
    cp \
    "${OUT_DIR}"/target/product/generic_arm64/obj/ETC/"${t}"_intermediates/${t}.apex \
    system/apex/tests/testdata/sharedlibs/
    ls -l system/apex/tests/testdata/sharedlibs/${t}.apex
done

for t in "${GENRULE_TARGETS[@]}"; do
    IFS=: read -a ar <<< "${t}"
    echo ar[0]
    echo ar[1]
    cp "${OUT_DIR}"/soong/.intermediates/"${ar[0]}"/"${ar[1]}"/gen/"${ar[1]}".apex \
    system/apex/tests/testdata/sharedlibs/
    ls -l system/apex/tests/testdata/sharedlibs/"${ar[1]}".apex
done


