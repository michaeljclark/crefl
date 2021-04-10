struct c {
	enum { one, two, three } t;
	int x;
	struct {
		_Complex double a;
		double b;
	} s;
};
struct c * v;
int __f (int a) { /* Do something. */; }
int f (int a) __attribute__ ((weak, alias ("__f")));
