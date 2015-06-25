// This is here because __builtin_apply and pals will not compile under egcs.
// better have derived the fn from Csym::functional()
void *__vcall(void (*fn)(), ...)
{
    if (fn) {
        void *result = __builtin_apply(fn,
                                       __builtin_apply_args(),
                                       256);
        __builtin_return(result);
    } else {
        return (void*)0;
    }
}
