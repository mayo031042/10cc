int for_loop()
{
    int i;
    for (i = 0; i < 10000; i += 1)
    {
        i;
        int x;
        continue;
    }
    return 1;
}

int nest()
{
    int ret;
    ret = 0;

    int x;
    x = 1;
    {
        {
            int x;
            x = 2;
            {
                int x;
                x = 3;
            }
            ret = x;
        }
    }
    return ret;
}

int cmnt_out()
{
    // return 2;
    /* /*
    return 2;
    */

    return 1;
}

int main()
{
    if (for_loop() != 1)
        return 1;
    if (nest() != 2)
        return 2;
    if (cmnt_out() != 1)
        return 3;

    return 0;
}