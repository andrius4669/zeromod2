
// checks message for various stuff
// returns: -1 in case return is needed, 0 for normal processing and 1 to skip message
static int messageintercept(int chan, int sender, clientinfo *ci, clientinfo *cq, int type, ...)
{
    if(!ci || ci->local) return 0;
    va_list vl;
    va_start(vl, type);
    int ret = 0;
    switch(type)
    {
        case N_TEXT:
        case N_SAYTEAM:
            if(ci->mute || (cq && cq->mute))
            {
                ret = 1;
                if(!ci->lastmutemsg || totalmillis-ci->lastmutemsg>=2000 || totalmillis-ci->lastmutemsg<0)
                {
                    toclient(ci, "your text message was muted");
                    ci->lastmutemsg  = totalmillis ? totalmillis : 1;
                }
            }
            break;

        case N_EDITF:
        case N_EDITT:
        case N_EDITM:
        case N_FLIP:
        case N_PASTE:
        case N_ROTATE:
        case N_REPLACE:
        case N_DELCUBE:
        case N_EDITENT:
            if(ci->editmute)
            {
                ret = 1;
                if(!ci->lasteditmutemsg || totalmillis-ci->lasteditmutemsg>=10000 || totalmillis-ci->lasteditmutemsg<0)
                {
                    toclient(ci, "your editing is muted");
                    ci->lasteditmutemsg = totalmillis ? totalmillis : 1;
                }
            }
            break;

        case N_COPY:
        case N_CLIPBOARD:
            break;  // allowed even if editmute

        case N_EDITMODE:
            // editmode cheat check
            if(!m_edit && (ci->clientmap[0] || ci->mapcrc)) { disconnect_client(sender, DISC_MSGERR); ret = -1; }
            break;

        case N_REMIP:
        case N_NEWMAP:
        case N_EDITVAR:
            if(ci->editmute)
            {
                ret = 1;
                toclientf(ci, "your %s was muted", type == N_REMIP ? "remip" : type == N_NEWMAP ? "newmap command" : "map variable edit");
            }
            break;

        case N_SPECTATOR:
        {
            int spectator = va_arg(vl, int), val = va_arg(vl, int);
            if(ci->specmute && sender == spectator && ci->privilege < PRIV_MASTER && !val)
            {
                ret = 1;
                toclient(ci, "you are not allowed to unspectate yourself");
            }
            break;
        }

        default:
            break;
    }
    va_end(vl);
    return ret;
}
