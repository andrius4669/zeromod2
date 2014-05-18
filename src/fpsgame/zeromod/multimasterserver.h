
extern int findauthmaster(const char *desc, int prev = -1);
extern int getmasterauthpriv(int m);
extern const char *getmasterauthdesc(int m);
extern const char *getmasteraddress(int m);
extern int nummasterservers();

#ifdef MULTIMASTERSERV_IMPLEMENTATION
// masterserver structure
struct masterserverinfo
{
    ENetSocket mastersock;
    ENetAddress masteraddress;
    int lastupdatemaster, lastconnectmaster, masterconnecting, masterconnected;
    vector<char> masterout, masterin;
    int masteroutpos, masterinpos;

    string mastername;
    int masterport;
    bool allowupdatemaster;

    bool allowauth, allowban;
    string authdesc;
    int authpriv;

    masterserverinfo(): mastersock(ENET_SOCKET_NULL),
        lastupdatemaster(0), lastconnectmaster(0),
        masterconnecting(0), masterconnected(0),
        masteroutpos(0), masterinpos(0),
        masterport(server::masterport()),
        allowupdatemaster(true),
        allowauth(true), allowban(true),
        authpriv(2)
    {
        copystring(mastername, server::defaultmaster());
        authdesc[0] = '\0';
        masteraddress.host = ENET_HOST_ANY;
        masteraddress.port = ENET_PORT_ANY;
    }
};

vector<masterserverinfo> masterservers;

int getcurrentmaster()
{
    if(masterservers.empty()) masterservers.add();
    return masterservers.length() - 1;
}

// functions to get masterservers parameters from other modules
int findauthmaster(const char *desc, int prev)
{
    for(int m = prev >= 0 ? prev + 1 : 0; m < masterservers.length(); m++)
        if(masterservers[m].allowauth && !strcmp(desc, masterservers[m].authdesc)) return m;
    return -1;
}

int getmasterauthpriv(int m)
{
    return masterservers[m].allowauth ? masterservers[m].authpriv : 0;
}

const char *getmasterauthdesc(int m)
{
    return masterservers[m].authdesc;
}

bool usemasterbans(int m)
{
    return masterservers[m].allowban;
}

const char *getmasteraddress(int m)
{
    static string buf;
    if(masterservers[m].masterport == server::masterport()) return masterservers[m].mastername;
    formatstring(buf)("%s:%d", masterservers[m].mastername, masterservers[m].masterport);
    return buf;
}

int nummasterservers() { return masterservers.length(); }

extern void disconnectmaster(int m = -1);
// commands for dealing with masterserver

ICOMMAND(clearmasterservers, "", (), { disconnectmaster(); masterservers.shrink(0); });
ICOMMAND(addmasterserver, "", (), masterservers.add());
ICOMMAND(nummasterservers, "", (), intret(masterservers.length()));

ICOMMAND(mastername, "s", (const char *s),
{
    int m = getcurrentmaster();
    disconnectmaster(m);
    masterservers[m].lastconnectmaster = 0;
    copystring(masterservers[m].mastername, *s ? s : server::defaultmaster());
});

ICOMMAND(masterport, "i", (const int *i),
{
    int m = getcurrentmaster();
    disconnectmaster(m);
    masterservers[m].lastconnectmaster = 0;
    masterservers[m].masterport = clamp(*i, 1, 0xFFFF);
});

ICOMMAND(updatemaster, "i", (const int *i),
{
    int m = getcurrentmaster();
    masterservers[m].allowupdatemaster = clamp(*i, 0, 1)!=0;
});

ICOMMAND(masterauth, "i", (const int *i),
{
    int m = getcurrentmaster();
    masterservers[m].allowauth = clamp(*i, 0, 1)!=0;
});

ICOMMAND(masterauthdesc, "s", (const char *s),
{
    int m = getcurrentmaster();
    copystring(masterservers[m].authdesc, s);
});

ICOMMAND(masterauthpriv, "s", (const char *s),
{
    int m = getcurrentmaster();
    int priv;
    switch(*s)
    {
        case 'a': case 'A': case '3': priv = 3; break;
        case '1': priv = 1; break;
        case 'c': case 'C': case '0': priv = 0; break;    // connect
        case 'm': case 'M': case '2': default: priv = 2; break;
    }
    masterservers[m].authpriv = priv;
});

ICOMMAND(masterban, "i", (const int *i),
{
    int m = getcurrentmaster();
    masterservers[m].allowban = clamp(*i, 0, 1)!=0;
});
#endif
