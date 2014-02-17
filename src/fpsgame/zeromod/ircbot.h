
ENetSocket ircsock = ENET_SOCKET_NULL;
ENetAddress ircaddress = { ENET_HOST_ANY, ENET_PORT_ANY };
vector<char> ircout, ircin;
int ircconnecting = ircconnected = irclastconnect = 0;
// ircconnecting and ircconnected are used when connecting, irclastconnect is used when reconnecting
int ircregistering = ircregistered = 0;
// ircregistering and ircregistered are used when bot registers to irc server

void disconnectirc()
{
    if(ircsock != ENET_SOCKET_NULL) { enet_socket_destroy(ircsock); ircsock = ENET_SOCKET_NULL; }
    ircout.setsize(0);
    ircin.setsize(0);
    ircaddress.host = ENET_HOST_ANY;
    ircaddress.port = ENET_PORT_ANY;
    ircconnecting = ircconnected = 0;
    ircregistering = ircregistered = 0;
}

SVARF(ircname, "irc.freenode.net", disconnectirc());
VARF(ircport, 1, 6667, 0xFFFF, disconnectirc());

ENetSocket connectirc()
{
    if(!ircname[0]) return ENET_SOCKET_NULL;
    if(ircaddress.host == ENET_HOST_ANY)
    {
        if(isdedicatedserver()) logoutf("looking up %s...", ircname);
        ircaddress.port = ircport;
        if(!resolverwait(ircname, &ircaddress)) return ENET_SOCKET_NULL;
    }
    ENetSocket sock = enet_socket_create(ENET_SOCKET_TYPE_STREAM);
    if(sock == ENET_SOCKET_NULL)
    {
        if(isdedicatedserver()) logoutf("could not open irc server socket");
        return ENET_SOCKET_NULL;
    }
    enet_socket_set_option(sock, ENET_SOCKOPT_NONBLOCK, 1);
    if(!enet_socket_connect(sock, &ircaddress))
    {
        ircconnecting = totalmillis ? totalmillis : 1;
        return sock;
    }
    enet_socket_destroy(sock);
    if(isdedicatedserver()) logoutf("could not connect to irc server");
    return ENET_SOCKET_NULL;
}

bool ircwrite(const char *s)
{
    if(ircsock == ENET_SOCKET_NULL) return false;
    if(ircout.length() >= 8192) return false;
    ircout.put(s, strlen(s));
    ircout.add('\r'); ircout.add('\n');
}

bool ircwritef(const char *fmt, ...)
{
    defvformatstring(s, fmt, fmt);
    return ircwrite(s);
}

bool ircregister()
{
    // connection and registration checks are done before calling this
    bool r = ircwrite("NICK testnick__") && ircwritef("USER  testuser__ testhost__ %s :real testing name", ircname);
    if(r) ircregistering = totalmillis ? totalmillis : 1;
    return r;
}

extern void ircleavechan();
SVAR(ircchan, "#devclan", ircleavechan());

void ircleavechan()
{
    
}

bool ircjoin()
{

}

void updateircserver()
{
    // first check connection settings
    if(ircsock == ENET_SOCKET_NULL && (!irclastconnect || totalmillis-irclastconnect>=15000))
    {
        ircsock = connectirc();
        irclastconnect = totalmillis ? totalmillis : 1;
        if(ircsock == ENET_SOCKET_NULL) return;
    }
    if(ircsock == ENET_SOCKET_NULL || !ircconnected)
    {
        if(ircconnecting && totalmillis-ircconnecting>=15000) disconnectirc();
        return;
    }
    // then registration
    if(!ircregistered)
    {
        if(ircregistering) { if(totalmillis-ircregistering>=15000) disconnectirc(); }
        else { if(!ircregister()) disconnectirc(); }
        return;
    }
    // then channel, if exists
    if
}




enum { IHS_DISCONNECTED = 0, IHS_CONNECTING, IHS_CONNECTED, IHS_REGISTERING, IHS_REGISTERED };
enum { ICS_LEFT = 0, ICS_JOINED };

// class for low level irc connection and data management
struct irchostconnector
{
    ENetSocket sock;
    ENetAddress address;
    vector<char> outbuf, inbuf;

    irchostconnector(): sock(ENET_SOCKET_NULL)
    {
        address.host = ENET_HOST_ANY;
        address.port = ENET_PORT_ANY;
    }

    bool connect(const char *host, int port)
    {
        if(!host[0]) return false;
        address.port = port;
        if(address.host == ENET_HOST_ANY)
        {
            if(isdedicatedserver()) logoutf("looking up %s...", host);
            if(!resolverwait(host, &address)) return false;
        }
        ENetSock esock = enet_socket_create(ENET_SOCKET_TYPE_STREAM);
        if(esock == ENET_SOCKET_NULL)
        {
            if(isdedicatedserver()) logoutf("could not open irc server socket");
            return ENET_SOCKET_NULL;
        }
        enet_socket_set_option(esock, ENET_SOCKOPT_NONBLOCK, 1);
        if(!enet_socket_connect(esock, &address)) { sock = esock; return true; }
        enet_socket_destroy(esock);
        if(isdedicatedserver()) logoutf("could not connect to irc server");
        return false;
    }

    void disconnect()
    {
        if(sock != ENET_SOCKET_NULL) { enet_socket_destroy(sock); sock = ENET_SOCKET_NULL; }
        outbuf.setsize(0);
        inbuf.setsize(0);
        address.host = ENET_HOST_ANY;
        address.port = ENET_PORT_ANY;
    }

    bool write(const char *s)
    {
        if(sock == ENET_SOCKET_NULL) return false;
        if(outbuf.length() >= 8192) return false;
        outbuf.put(s, strlen(s));
        return true;
    }

    bool writef(const char *fmt, ...)
    {
        

    bool writeln(const char *s)
    {
        if(!write(s)) return false;
        outbuf.add('\r'); outbuf.add('\n');
    }
};

struct irchost
{
    int state;
    int statemillis;
    string host;
    int port;
    string nick;
    irchostconnector connector;

    irchost(): state(IHS_DISCONNECTED), statemillis(0) {}

    void updatestate(int newstate) { state = newstate; statemillis = totalmillis; }

    void check()
    {
        switch(state)
        {
            case IHS_DISCONNECTED:
                if((!statemillis || totalmillis-statemillis>=5000) && connector.connect()) updatestate(IHS_CONNECTING);
                break;

            case IHS_CONNECTING:
                break;

            case IHS_CONNECTED:
                connector.writeln("
        }
    }
}