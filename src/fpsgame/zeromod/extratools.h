// functions not depending on clientinfo struct

/* based on readban from engine/master.cpp */
void readban(const char *name, enet_uint32 *cip, enet_uint32 *cmask)
{
    union { uchar b[sizeof(enet_uint32)]; enet_uint32 i; } ip, mask;
    ip.i = 0;
    mask.i = 0;
    const char *cidr = strchr(name, '/');
    loopi(4)
    {
        char *end = NULL;
        int n = strtol(name, &end, 10);
        if(!end) break;
        if(end > name) { ip.b[i] = n; mask.b[i] = 0xFF; }
        name = end;
        while(*name && *name++ != '.');
    }
    if(cidr && *++cidr)
    {
        int n = atoi(cidr);
        if(n > 0)
        {
            for(int i = n; i < 32; i++) mask.b[i/8] &= ~(1 << (7 - (i & 7)));
            ip.i &= mask.i;
        }
    }
    if(cip) *cip = ip.i;
    if(cmask) *cmask = mask.i;
}

/* based on addgban from fpsgame/server.cpp */
static char *printban(enet_uint32 *ip_, enet_uint32 *mask_, char *buf, bool forcecidr = false)
{
    union { uchar b[sizeof(enet_uint32)]; enet_uint32 i; } ip, mask;
    ip.i = ip_ ? *ip_ : 0;
    mask.i = mask_ ? *mask_ : ~0;
    int lastdigit = -1;
    bool needcidr = forcecidr;
    loopi(4) if(mask.b[i] || needcidr)
    {
        if(mask.b[i]!=0xFF) needcidr = true;
        if(lastdigit >= 0) *buf++ = '.';
        loopj(i - lastdigit - 1) { *buf++ = '*'; *buf++ = '.'; }
        buf += sprintf(buf, "%d", ip.b[i]);
        lastdigit = i;
    }
    if(needcidr)
    {
        int c = 32;
        while(((--c) >= 0) && !(mask.b[c/8] & (1 << (7 - (c&0x7)))));
        c++;
        *buf++ = '/';
        buf += sprintf(buf, "%d", c);
    }
    return buf;
}

static int readbantime(char *s)
{
    int t = 0;
    for(;;)
    {
        char *end = NULL;
        int n = strtol(s, &end, 10);
        if(!end) break;
        if(end <= s) n = 1;
        switch(*end++)
        {
            case 'w': case 'W': n *= 7;     // fall through
            case 'd': case 'D': n *= 24;    // fall through
            case ' ': case '\0':
            case 'h': case 'H': n *= 60;    // fall through
            case 'm': case 'M': n *= 60;    // fall through
            case 's': case 'S': n *= 1000; break;
            default: n = 0; break;
        }
        s = end;
        t += n;
    }
    return t;
}
