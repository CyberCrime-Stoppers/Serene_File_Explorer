#include "clipboard.h"

Clipboard &Clipboard::instance()
{
    static Clipboard inst;
    return inst;
}

void Clipboard::clear()
{
    mode = None;
    paths.clear();
}

bool Clipboard::isEmpty() const
{
    return mode == None || paths.isEmpty();
}
