#include <stdio.h>

// example target program: uses every feature you will add
const double _ratio = 1.5;

int max(int a, int b) {
    if (a > b)
        return a;
    else
        return b;
}

int _clamp(int _v, int _lo) {
    int _r = _v * 2 - _lo;
    _r -= 1;
    _r *= 3;
    _r /= 2;
    _r = _r % 5;
    if (_r == _lo || !(_r <= 10)) {
        return -1;
    }
    return _r;
}

int main(void) {
    int i, n, sum;
    float avg;
    char grade;
    /* block comments
       may span lines */
    n = 10;
    sum = 0;
    i = 0;
    while (i < n) {
        sum += i;
        i++;
    }
    for (i = 0; i < 5; i++) {
        sum = sum + i;
        if (i == 2) {
            continue;
        }
    }
    do {
        n--;
    } while (n > 0);
    avg = sum / 3.0;
    avg = avg * _ratio - .5;
    grade = 'A';
    if ((avg >= 10.0) && (grade != 'F')) {
        printf("pass\n");
    } else {
        printf("fail\n");
    }
    switch (grade) {
        case 'A':
            printf("excellent\n");
            break;
        default:
            printf("ok\n");
    }
    sum = _clamp(sum, 0);
    printf("sum=%d\n", max(sum, 100));
    return 0;
}
