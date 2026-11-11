#include "cli.h"
#include "ui.h"

int main(void)
{
    if (ui_run()) {
        return 0;
    }
    return cli_run();
}
