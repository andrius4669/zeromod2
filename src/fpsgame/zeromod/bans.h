/* header for pbans management and some gbans server commands */

void formatgban(char *s, int n, bool forcecidr = false)
{
    union { uchar b[sizeof(enet_uint32)]; enet_uint32 i; } ip, mask;
    ip.i = gbans[n].ip;
    mask.i = gbans[n].mask;
    bool needcidr = forcecidr;
    if(!forcecidr) loopi(4) if(mask.b[i]!=0 && mask.b[i]!=0xFF) { needcidr = true; break; }
    loopi(4)
    {
        if(!mask.b[i] && !needcidr) break;
        if(i) *s++ =  '.';
        s += sprintf(s, "%d", ip.b[i]);
    }
    if(needcidr)
    {
        int c = 0;
        loopi(32) if(mask.b[i>>3] & (1<<(7-(i&7)))) c++;
        if(c < 32)
        {
            *s++ = '/';
            sprintf(s, "%d", c);
        }
    }
}

void scmd_listpbans(int argc, char **argv, clientinfo *ci)
{
    int n = 0;
    toclient(ci, "pbans list:");
    string msg;
    loopv(gbans) if(gbans[i].master < 0)
    {
        formatstring(msg)("%i ", ++n);
        formatgban(msg + strlen(msg), i, true);
        toclient(ci, msg);
    }
    if(!n) toclient(ci, "none");
}
SCOMMAND(listpbans, scmd_listpbans, PRIV_ADMIN, 0);

void scmd_pban(int argc, char **argv, clientinfo *actor)
{
    if(argc < 2) { scommandbadusage(actor, argv[0]); return; }
    int cn = atoi(argv[1]);
    if(!cn && strcmp(argv[1], "0")) { scommandbadusage(actor, argv[0]); return; }
    clientinfo *cx = (clientinfo *)getclientinfo(cn);
    if(!cx) { scommandbadcn(actor, cn); return; }
    if(cx->local) { toclient(actor, "cannot pban local player"); return; }
    toclientf(actor, "adding pban for %s", getclienthostname(cn));
    uint ip = getclientip(cn);
    gbaninfo &ban = gbans.add();
    ban.ip = ip;
    ban.mask = ~0;
    ban.master = -1;
    loopvrev(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->state.aitype != AI_NONE || ci->local || ci->privilege >= PRIV_ADMIN || ci->clientnum == actor->clientnum) continue;
        if(checkgban(getclientip(ci->clientnum))) disconnect_client(ci->clientnum, DISC_IPBAN);
    }
}
SCOMMAND(pban, scmd_pban, PRIV_ADMIN, 0);

void scmd_pbanip(int argc, char **argv, clientinfo *ci)
{
    if(argc < 2) { scommandbadusage(ci, argv[0]); return; }
    toclientf(ci, "adding pban for %s", argv[1]);
    addgban(-1, argv[1], ci);
}
SCOMMAND(pbanip, scmd_pbanip, PRIV_ADMIN, 0);

ICOMMAND(clearpbans, "", (), { loopvrev(gbans) if(gbans[i].master < 0) gbans.removeunordered(i); });
ICOMMAND(pban, "s", (const char *s), addgban(-1, s));

void scmd_unpban(int argc, char **argv, clientinfo *ci)
{
    static char buf[100] = "removing pban for ";
    static int bufpos = 0;
    if(!bufpos) bufpos = strlen(buf);
    int n = 0;
    if(argc < 2) { scommandbadusage(ci, argv[1]); return; }
    int m = atoi(argv[1]);
    if(!m && strcmp(argv[1], "0")) { scommandbadusage(ci, argv[1]); return; }
    bool found = false;
    loopv(gbans) if(gbans[i].master < 0 && (m <= 0 || ++n == m))
    {
        formatgban(buf + bufpos, i, true);
        toclient(ci, buf);
        gbans.remove(i--);
        found = true;
    }
    if(!found && m > 0) toclient(ci, "pban doesn't exist");
}
SCOMMAND(unpban, scmd_unpban, PRIV_ADMIN, 0);

void scmd_listgbans(int argc, char **argv, clientinfo *ci)
{
    vector< vector<int> > gbansi;
    string msg;
    loopv(gbans)
    {
        int m = gbans[i].master;
        if(m < 0) continue;
        while(m >= gbansi.length()) gbansi.add();
        gbansi[m].add(i);
    }
    loopv(gbansi)
    {
        toclientf(ci, "masterserver (%s) gbans:", getmasteraddress(i));
        if(gbansi[i].empty()) toclient(ci, "none");
        loopvj(gbansi[i])
        {
            formatgban(msg, gbansi[i][j]);
            toclientf(ci, msg);
        }
    }
}
SCOMMAND(listgbans, scmd_listgbans, PRIV_ADMIN, 0);
