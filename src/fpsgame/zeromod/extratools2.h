// functions which depends on clientinfo struct

void toclient(clientinfo *ci, const char *s) { sendf(ci ? ci->clientnum : -1, 1, "ris", N_SERVMSG, s); }
void toclientf(clientinfo *ci, const char *fmt, ...)
{
    defvformatstring(s, fmt, fmt);
    sendf(ci ? ci->clientnum : -1, 1, "ris", N_SERVMSG, s);
}
