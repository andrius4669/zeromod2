
#ifdef SCMD_HEAD

typedef void (* scommandfun)(int argc, char **argv, clientinfo *ci);

struct scommandstruct
{
    string name;
    scommandfun fun;
    int priv;
    int numargs;
    bool hidden;

    scommandstruct(const char *n, scommandfun f, int p, int a, bool h): fun(f), priv(p), numargs(a), hidden(h) { copystring(name, n); }
};

extern void addscommand(scommandstruct *c);
extern void delscommand(scommandstruct *c);

struct scommandar: scommandstruct
{
    scommandar(const char *n, scommandfun f, int p, int a, bool h): scommandstruct(n, f, p, a, h) { addscommand(this); }
    ~scommandar() { delscommand(this); }
};

#define SCOMMAND(n, f, p, a) scommandar _scmd_##n(#n, f, p, a, false);
#define SCOMMANDH(n, f, p, a) scommandar _scmd_##n(#n, f, p, a, true);

extern void scommandprivfail(clientinfo *ci, scommandstruct *cc);
extern void scommandnotfound(clientinfo *ci, const char *cmd);
extern void scommandbadusage(clientinfo *ci, const char *cmd);
extern void scommandbadcn(clientinfo *ci, int cn);

#endif

#ifdef SCMD_FOOT

vector<scommandstruct *> scommands;
static bool scommandsinit = true;
static vector<scommandstruct *> *iscommands = NULL;

void addscommand(scommandstruct *c)
{
    if(scommandsinit)
    {
        if(!iscommands) iscommands = new vector<scommandstruct *>;
        iscommands->add(c);
        return;
    }
    scommands.add(c);
}

void delscommand(scommandstruct *c)
{
    if(iscommands) iscommands->removeobj(c);
    if(!scommandsinit) scommands.removeobj(c);
}

void initscommands()
{
    scommandsinit = false;
    if(iscommands)
    {
        loopv(*iscommands) scommands.add((*iscommands)[i]);
        DELETEP(iscommands);
    }
}

SVAR(commandchars, "#");

void scommandnotfound(clientinfo *ci, const char *cmd)
{
    defformatstring(msg)("unknown server command: %s", cmd);
    toclient(ci, msg);
    static int lastcmdnf = 0;
    if(!lastcmdnf || totalmillis-lastcmdnf>=250 || totalmillis-lastcmdnf<0)
    {
        lastcmdnf = totalmillis;
        loopv(scommands) if(!scommands[i]->hidden && (!strcasecmp(scommands[i]->name, cmd) || (strlen(cmd) >= 3 && strstr(scommands[i]->name, cmd)) || strstr(cmd, scommands[i]->name)))
        {
            formatstring(msg)("maybe you meant \fs\f0%s\fr?", scommands[i]->name);
            toclient(ci, msg);
            return;
        }
    }
}

void scommandprivfail(clientinfo *ci, scommandstruct *cc)
{
    toclient(ci, "your privilege is too low to execute this command");
}

void scommandbadusage(clientinfo *ci, const char *cmd)
{
    toclient(ci, "incorrect command usage. try to use \fs\f0help\fr command to obtain usage reference.");
}

void scommandbadcn(clientinfo *ci, int cn)
{
    toclientf(ci, "client with number \fs\f0%d\fr not found", cn);
}

bool parsecommand(clientinfo *ci, char *cmd, bool iscommand)
{
    /*char cmdchar = '\0';*/
    initscommands();
    if(!cmd[0]) return false;
    if(!iscommand)
    {
        const char *c = commandchars;
        while(*c && *c != cmd[0]) c++;
        if(!*c) return false;
        /*cmdchar = *c;*/
        cmd++;
    }
    int argc = 1;
    char *argv[24];
    argv[0] = cmd;
    for(int i = 1; i < 24; i++) argv[i] = NULL;
    char *s = strchr(cmd, ' ');
    if(s)
    {
        *s++ = '\0';
        while(*s == ' ') s++;
        if(*s) { argv[1] = s; argc++; }
    }
    scommandstruct *cc = NULL;
    loopv(scommands) if(!strcmp(scommands[i]->name, cmd)) { cc = scommands[i]; break; }
    if(!cc) { scommandnotfound(ci, cmd); return true; }
    if(!ci->local && ci->privilege < cc->priv) { scommandprivfail(ci, cc); return true; }
    int n = cc->numargs ? min(cc->numargs+1, 24) : 24;
    if(argv[1])
    {
        for(int i = 2; i < n; i++)
        {
            s = strchr(argv[i - 1], ' ');
            if(!s) break;
            *s++ = '\0';
            while(*s == ' ') s++;
            if(!*s) break;
            argv[i] = s;
            argc++;
        }
    }
    cc->fun(argc, argv, ci);
    return true;
}

#endif
