static const int iRotPtn[] = { 0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0, 0 };
//割り込みで変化を読み取る
void interrupt_enc(void) {
  byte cur1 = (!digitalRead(SigB1) << 1) + !digitalRead(SigA1);
  byte old1 = pos1 & 0b00000011;
  byte dir1 = (pos1 & 0b00110000) >> 4;

  if (cur1 == 3) cur1 = 2;
  else if (cur1 == 2) cur1 = 3;

  if (cur1 != old1) {
    if (dir1 == 0) {
      if (cur1 == 1 || cur1 == 3) dir1 = cur1;
    } else {
      if (cur1 == 0) {
        if (dir1 == 1 && old1 == 3) enc_count1++;
        else if (dir1 == 3 && old1 == 1) enc_count1--;
        dir1 = 0;
      }
    }

    bool rote1 = 0;
    if (cur1 == 3 && old1 == 0) rote1 = 0;
    else if (cur1 == 0 && old1 == 3) rote1 = 1;
    else if (cur1 > old1) rote1 = 1;

    pos1 = (dir1 << 4) + (old1 << 2) + cur1;
  }
}

void interrupt_enc2(void) {
  byte cur2 = (!digitalRead(SigB2) << 1) + !digitalRead(SigA2);
  byte old2 = pos2 & 0b00000011;
  byte dir2 = (pos2 & 0b00110000) >> 4;

  if (cur2 == 3) cur2 = 2;
  else if (cur2 == 2) cur2 = 3;

  if (cur2 != old2) {
    if (dir2 == 0) {
      if (cur2 == 1 || cur2 == 3) dir2 = cur2;
    } else {
      if (cur2 == 0) {
        if (dir2 == 1 && old2 == 3) enc_count2++;
        else if (dir2 == 3 && old2 == 1) enc_count2--;
        dir2 = 0;
      }
    }

    bool rote2 = 0;
    if (cur2 == 3 && old2 == 0) rote2 = 0;
    else if (cur2 == 0 && old2 == 3) rote2 = 1;
    else if (cur2 > old2) rote2 = 1;

    pos2 = (dir2 << 4) + (old2 << 2) + cur2;
  }
}