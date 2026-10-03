Mini Project 1 examples

Extract this folder beside front.c and front.in. Use Bash on Linux or WSL:
    gcc -std=c11 -Wall -Wextra front.c -o front
    ./front

basic/front.in: the expression the starter already recognizes.
basic/front.stdout and .stderr: correct output for that expression.
target/target.c: the complete target program shown in the handout.
starter/target.stdout and .stderr: output BEFORE implementing the assignment.
expected/target.stdout and .stderr: output AFTER completing the scanner.
recovery/recovery.in: a malformed float followed by a valid assignment.
recovery/recovery.stdout and .stderr: completed-scanner output for recovery.in.

Check your completed scanner:
    ./front examples/target/target.c > my_stdout.txt 2> my_stderr.txt

You can use `diff` to compare the difference.
    diff my_stdout.txt examples/expected/target.stdout
    diff my_stderr.txt examples/expected/target.stderr

Both comparisons should show no differences. The starter does not yet pass
this comparison. Errors and missing token types in its target output are expected.

The target is scanner INPUT, not a file you need to compile for the assignment.
It covers every required non-error token code, both comment forms, and # skipping.
Recovery output contains an error token and diagnostic, then continues to EOF.
Empty .stderr files are intentional. Preserve the LF line endings in these files.
