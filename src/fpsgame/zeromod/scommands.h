
#ifndef __SCOMMANDS_H__
#define __SCOMMANDS_H__ 1

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

// class which automatically registers commands
struct scommandar: scommandstruct
{
    scommandar(const char *n, scommandfun f, int p, int a, bool h): scommandstruct(n, f, p, a, h) { addscommand(this); }
    ~scommandar() { delscommand(this); }
};

#define SCOMMAND(n, f, p, a) scommandar _scmd_##n(#n, f, p, a, false);
#define SCOMMANDH(n, f, p, a) scommandar _scmd_##n(#n, f, p, a, true);
#define SCOMMANDI(n, p, a, body) void _scmdf_##n(int argc, char **argv, clientinfo *ci) { body; } \
                    scommandar _scmd_##n(#n, _scmdf_##n, p, a, false);
#define SCOMMANDIH(n, p, a, body) void _scmdf_##n(int argc, char **argv, clientinfo *ci) { body; } \
                    scommandar _scmd_##n(#n, _scmdf_##n, p, a, true);

extern void scommandprivfail(clientinfo *ci, scommandstruct *cc);
extern void scommandnotfound(clientinfo *ci, const char *cmd);
extern void scommandbadusage(clientinfo *ci, const char *cmd);
extern void scommandbadcn(clientinfo *ci, int cn);
extern clientinfo *findclient(const char *s, bool allowbots = true, clientinfo *caller = NULL);

#endif
