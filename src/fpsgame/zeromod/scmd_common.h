void scmd_hidepriv(int argc, char **argv, clientinfo *ci)
{
    ci->nexthidepriv = !ci->nexthidepriv;
    toclientf(ci, "next claim will be %s", ci->nexthidepriv ? "hidden" : "visible");
}
SCOMMANDH(hidepriv, scmd_hidepriv, 0, 0);

void scmd_nodamage(int argc, char **argv, clientinfo *ci)
{
    int m = -1;
    if(argc>=2) switch(argv[1][0]) { case '0': m = 0; break; case '1': m = 1; break; }
    if(m >= 0) { if(nodamage == m || nodamage < 0) m = -1; else nodamage = m; }
    toclientf(m < 0 ? ci : NULL, "nodamage %s", nodamage>0 ? "enabled" : nodamage<0 ? "disabled in server configuration" : "disabled");
}
SCOMMAND(nodamage, scmd_nodamage, PRIV_MASTER, 0);
