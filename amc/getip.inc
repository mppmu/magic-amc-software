    pf = (*ptr).portflg[it];
    ip = (*ptr).port[0][it];
    if(pf != 0) { pf=1; ip = (*ptr).port[pf][it]+1; }

