
void il_logger_init();

int il_bootstrap(int argc, char **argv) 
{
    (void)argc, (void)argv;
    il_logger_init();
    return 0;
}

