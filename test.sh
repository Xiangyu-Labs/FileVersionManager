#!/usr/bin/env bash
#
# Isolated regression tests for FileVersionManager.
# All compilation and execution happens inside a mktemp directory, so no
# data.chm / log.chm is ever created inside the repository.

ROOT="$(cd "$(dirname "$0")" && pwd)"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
cd "$WORK"

FAILURES=0

fail() {
    echo "FAIL: $1"
    FAILURES=$((FAILURES + 1))
}

ok() {
    echo "ok: $1"
}

expect_exit() {
    local name="$1" expected="$2"
    local got
    got="$(cat "exit_${name}.txt")"
    if [ "$got" != "$expected" ]; then
        fail "$name: expected exit code $expected, got $got"
    else
        ok "$name: exit code $expected"
    fi
}

expect_contains() {
    local name="$1" file="$2" pattern="$3"
    if ! grep -qF -- "$pattern" "$file"; then
        fail "$name: expected '$file' to contain '$pattern'"
    else
        ok "$name: contains '$pattern'"
    fi
}

expect_not_contains() {
    local name="$1" file="$2" pattern="$3"
    if grep -qF -- "$pattern" "$file"; then
        fail "$name: expected '$file' NOT to contain '$pattern'"
    else
        ok "$name: does not contain '$pattern'"
    fi
}

# Run the binary in an isolated subdirectory with a fixed input string;
# capture stdout, stderr and exit code. Tests that must share persisted
# state use the same directory name.
run_with_input() {
    local name="$1" dir="$2" input="$3"
    mkdir -p "$WORK/$dir"
    printf '%s' "$input" | (cd "$WORK/$dir" && "$WORK/fvm") >"out_${name}.txt" 2>"err_${name}.txt"
    echo "$?" >"exit_${name}.txt"
}

# ---------------------------------------------------------------------------
echo "== Test 1: compile with clang++ -std=c++11 -Wall -Wextra -Wpedantic =="
if command -v clang++ >/dev/null 2>&1; then
    clang++ -std=c++11 -Wall -Wextra -Wpedantic "$ROOT/main.cpp" -o fvm 2>compile_warnings.txt
    if [ $? -ne 0 ]; then
        fail "compile: clang++ failed"
    else
        ok "compile: succeeded"
        if grep -q "does not append to the string" compile_warnings.txt; then
            fail "compile: string pointer offset warning (-Wstring-plus-int) still present"
        else
            ok "compile: no string pointer offset warning"
        fi
        if grep -q "expression result unused" compile_warnings.txt; then
            fail "compile: unused expression warning (-Wunused-value) still present"
        else
            ok "compile: no unused expression warning"
        fi
    fi
else
    fail "compile: clang++ not available"
fi

# ---------------------------------------------------------------------------
echo "== Test 2: basic file system operations =="
run_with_input t2 t2 'touch a
update_content a hello
cat a
update_name a b
cat b
mkdir d
rmf b
cat b
cd d
pwd
cdl
pwd
rmd d
ls
exit
'
expect_exit t2 0
expect_contains t2 out_t2.txt "hello"
expect_contains t2 out_t2.txt "/root/d/"
expect_contains t2 out_t2.txt "/root/"
expect_contains t2 out_t2.txt "no file or directory named b"
expect_contains t2 out_t2.txt "The folder is empty"

# ---------------------------------------------------------------------------
echo "== Test 3: update_content on a missing target must not touch other files =="
run_with_input t3 t3 'touch f
update_content f original
update_content missing changed
cat f
exit
'
expect_exit t3 0
expect_contains t3 out_t3.txt "no file or directory named missing"
expect_contains t3 out_t3.txt "original"
expect_not_contains t3 out_t3.txt "changed"

# ---------------------------------------------------------------------------
echo "== Test 4: version inheritance keeps versions isolated (incl. restart) =="
run_with_input t4a t4 'touch f
update_content f AAA
create_version 1001
update_content f BBB
cat f
switch_version 1001
cat f
switch_version 1002
cat f
exit
'
expect_exit t4a 0
expect_contains t4a out_t4a.txt "AAA"
expect_contains t4a out_t4a.txt "BBB"

run_with_input t4b t4 'switch_version 1001
cat f
switch_version 1002
cat f
exit
'
expect_exit t4b 0
expect_contains t4b out_t4b.txt "AAA"
expect_contains t4b out_t4b.txt "BBB"

# ---------------------------------------------------------------------------
echo "== Test 5: rename and content update keep create time, refresh update time =="
mkdir -p "$WORK/t5"
(
    printf 'touch f\nls -a\n'
    sleep 1
    printf 'update_name f g\nls -a\n'
    sleep 1
    printf 'update_content g hello\nls -a\nexit\n'
) | (cd "$WORK/t5" && "$WORK/fvm") >out_t5.txt 2>err_t5.txt
echo "$?" >exit_t5.txt
expect_exit t5 0

time_line1="$(grep '^file' out_t5.txt | sed -n '1p')"
time_line2="$(grep '^file' out_t5.txt | sed -n '2p')"
time_line3="$(grep '^file' out_t5.txt | sed -n '3p')"
if [ -z "$time_line1" ] || [ -z "$time_line2" ] || [ -z "$time_line3" ]; then
    fail "t5: could not find three ls -a lines"
else
    ok "t5: found three ls -a lines"
fi

create1="$(printf '%s' "$time_line1" | awk -F'\t' '{print $2}')"
update1="$(printf '%s' "$time_line1" | awk -F'\t' '{print $3}')"
create2="$(printf '%s' "$time_line2" | awk -F'\t' '{print $2}')"
update2="$(printf '%s' "$time_line2" | awk -F'\t' '{print $3}')"
create3="$(printf '%s' "$time_line3" | awk -F'\t' '{print $2}')"
update3="$(printf '%s' "$time_line3" | awk -F'\t' '{print $3}')"

valid_time() {
    printf '%s' "$1" | grep -Eq '^[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}$'
}

if valid_time "$create1" && valid_time "$update1" && valid_time "$create2" \
    && valid_time "$update2" && valid_time "$create3" && valid_time "$update3"; then
    ok "t5: all timestamps match YYYY-MM-DD HH:MM:SS"
else
    fail "t5: timestamp format invalid"
fi

hour_ok=1
for t in "$create1" "$update1" "$create2" "$update2" "$create3" "$update3"; do
    hour="$(printf '%s' "$t" | cut -d' ' -f2 | cut -d: -f1)"
    minute="$(printf '%s' "$t" | cut -d' ' -f2 | cut -d: -f2)"
    second="$(printf '%s' "$t" | cut -d' ' -f2 | cut -d: -f3)"
    if [ "$hour" -gt 23 ] || [ "$minute" -gt 59 ] || [ "$second" -gt 59 ]; then
        hour_ok=0
    fi
done
if [ "$hour_ok" -eq 1 ]; then
    ok "t5: all timestamps contain legal hour/minute/second values"
else
    fail "t5: illegal time value (e.g. 24:xx) detected"
fi

if [ "$create1" = "$create2" ] && [ "$create2" = "$create3" ]; then
    ok "t5: create time preserved across rename and content update"
else
    fail "t5: create time changed: $create1 -> $create2 -> $create3"
fi

if [ "$update1" != "$update2" ] && [ "$update2" != "$update3" ] \
    && [[ "$update2" > "$update1" ]] && [[ "$update3" > "$update2" ]]; then
    ok "t5: update time refreshed on rename and on content update"
else
    fail "t5: update time not refreshed: $update1 -> $update2 -> $update3"
fi

# ---------------------------------------------------------------------------
echo "== Test 6: init removes custom aliases and restores defaults =="
run_with_input t6 t6 'add_identifier myls 13
myls
delete_identifier ls
ls
init
myls
ls
exit
'
expect_exit t6 0
expect_contains t6 out_t6.txt "An identifier was successfully added for program 13."
count_empty="$(grep -c "The folder is empty" out_t6.txt)"
if [ "$count_empty" = "2" ]; then
    ok "t6: myls worked before init, was removed by init, ls was restored by init"
else
    fail "t6: expected exactly 2 folder-empty outputs, got $count_empty"
fi
expect_contains t6 "$WORK/t6/log.chm" "Command not found: myls"
expect_contains t6 "$WORK/t6/log.chm" "Command not found: ls"

# ---------------------------------------------------------------------------
echo "== Test 7: invalid arguments are rejected =="
run_with_input t7 t7 'touch a b
ls -x
ls a b
create_version abc def
version
add_identifier x 1234567890123456789
ls
exit
'
expect_exit t7 0
expect_contains t7 out_t7.txt "Too many parameters. 1 parameters were required but 2 were provided."
expect_contains t7 out_t7.txt 'Invalid parameter for ls: -x. Only "-a" is accepted.'
expect_contains t7 out_t7.txt "The ls command accepts at most 1 parameter, but 2 were provided."
expect_contains t7 out_t7.txt "At least one of the two parameters of create_version must be an integer version number."
expect_contains t7 out_t7.txt "The 1th argument has a maximum of 18."
expect_not_contains t7 out_t7.txt "1002"
expect_contains t7 out_t7.txt "The folder is empty"

# ---------------------------------------------------------------------------
echo "== Test 8: EOF on stdin exits with code 0 =="
mkdir -p "$WORK/t8a"
printf 'touch a\nls\n' | (cd "$WORK/t8a" && "$WORK/fvm") >out_t8.txt 2>err_t8.txt
echo "$?" >exit_t8.txt
expect_exit t8 0
expect_contains t8 out_t8.txt "a"

mkdir -p "$WORK/t8b"
printf 'touch b' | (cd "$WORK/t8b" && "$WORK/fvm") >out_t8b.txt 2>err_t8b.txt
echo "$?" >exit_t8b.txt
expect_exit t8b 0

mkdir -p "$WORK/t8c"
printf '' | (cd "$WORK/t8c" && "$WORK/fvm") >out_t8c.txt 2>err_t8c.txt
echo "$?" >exit_t8c.txt
expect_exit t8c 0

# ---------------------------------------------------------------------------
echo "== Test 9: first empty start, save, restart =="
run_with_input t9a t9 'gcv
exit
'
expect_exit t9a 0
expect_contains t9a out_t9a.txt "The current version of the file system is 1001"
if [ -f "$WORK/t9/data.chm" ] && [ -f "$WORK/t9/log.chm" ]; then
    ok "t9: data.chm and log.chm created in the isolated directory"
else
    fail "t9: data.chm or log.chm missing after first run"
fi

run_with_input t9b t9 'touch f
update_content f hello
cat f
exit
'
expect_exit t9b 0
expect_contains t9b out_t9b.txt "hello"

# ---------------------------------------------------------------------------
echo "== Test 10: truncated data.chm must not crash or be overwritten =="
run_with_input t10a t10 'touch f
update_content f hello
exit
'
expect_exit t10a 0
head -c 60 "$WORK/t10/data.chm" >"$WORK/data.trunc"
mv "$WORK/data.trunc" "$WORK/t10/data.chm"
checksum_before="$(cksum <"$WORK/t10/data.chm")"
run_with_input t10b t10 'gcv
exit
'
expect_exit t10b 0
checksum_after="$(cksum <"$WORK/t10/data.chm")"
if [ "$checksum_before" = "$checksum_after" ]; then
    ok "t10: corrupted data.chm checksum unchanged after the run"
else
    fail "t10: corrupted data.chm was modified"
fi

# ---------------------------------------------------------------------------
echo "== Test 11: AddressSanitizer + UndefinedBehaviorSanitizer workflow =="
clang++ -std=c++11 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined \
    -fno-omit-frame-pointer "$ROOT/main.cpp" -o fvm_san 2>san_compile_warnings.txt
if [ $? -ne 0 ]; then
    fail "sanitizer compile failed"
else
    ok "sanitizer compile: succeeded"
    mkdir -p "$WORK/san"
    (
        printf 'touch f\nupdate_content f AAA\nupdate_name f g\nmkdir d\ncd d\npwd\ncdl\n'
        printf 'create_version 1001\nupdate_content g BBB\nswitch_version 1001\ncat g\n'
        printf 'switch_version 1002\ncat g\nrmf g\nls -a\nadd_identifier myls 13\ninit\nrmd d\n'
        sleep 1
        printf 'create_version info 1002\nversion\ncreate_version x y\nls -x\ntouch a b\n'
        printf 'add_identifier z 12345678901234567890\nupdate_content missing changed\nexit\n'
    ) | (cd "$WORK/san" && "$WORK/fvm_san") >san_out.txt 2>san_err.txt
    echo "$?" >exit_san.txt
    expect_exit san 0
    (
        printf 'switch_version 1001\ncat g\nswitch_version 1002\ncat g\nls -a\nversion\nexit\n'
    ) | (cd "$WORK/san" && "$WORK/fvm_san") >san_out2.txt 2>san_err2.txt
    echo "$?" >exit_san2.txt
    expect_exit san2 0
    head -c 60 "$WORK/san/data.chm" >"$WORK/san.data.trunc"
    mv "$WORK/san.data.trunc" "$WORK/san/data.chm"
    printf 'gcv\nexit\n' | (cd "$WORK/san" && "$WORK/fvm_san") >san_out3.txt 2>san_err3.txt
    echo "$?" >exit_san3.txt
    expect_exit san3 0
    if grep -qE "AddressSanitizer|UndefinedBehaviorSanitizer|runtime error|use-after-free|heap-buffer-overflow|stack-buffer-overflow" san_err.txt san_err2.txt san_err3.txt; then
        fail "sanitizer reported an error"
    else
        ok "sanitizer: no errors reported"
    fi
fi

# ---------------------------------------------------------------------------
echo "== Repo isolation check =="
if [ -e "$ROOT/data.chm" ] || [ -e "$ROOT/log.chm" ]; then
    fail "isolation: data.chm or log.chm was created inside the repository"
else
    ok "isolation: no data.chm / log.chm inside the repository"
fi

echo
if [ "$FAILURES" -eq 0 ]; then
    echo "All tests passed."
    exit 0
else
    echo "$FAILURES test(s) failed."
    exit 1
fi
