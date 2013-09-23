void ilA_union_init();
void ilA_stdiofile_init();
void ilA_stdiodir_init();
char *strdup(const char*);
char *strtok_r(char *str, const char *delim, char **saveptr);
int il_bootstrap(int argc, char **argv)
{
    (void)argc, (void)argv;

    ilA_union_init();
    ilA_stdiofile_init();
    ilA_stdiodir_init();

    return 0;
}

