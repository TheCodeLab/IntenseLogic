void ilA_union_init();
void ilA_stdiofile_init();
void ilA_stdiodir_init();

int il_load_ilasset()
{
    ilA_union_init();
    ilA_stdiofile_init();
    ilA_stdiodir_init();

    return 0;
}

