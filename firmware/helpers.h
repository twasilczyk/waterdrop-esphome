static uint16_t counter = 0;

uint8_t counterHi() {
    if (++counter > 999) {
        counter = 0;
    }
    return counter >> 8;
}

uint8_t counterLo() {
    return counter & 0xFF;
}

uint8_t bflip(uint8_t byte, uint8_t byte_no, uint8_t flip_idx) {
    if (flip_idx == 0) return byte;
    flip_idx--;

    if (flip_idx / 8 != byte_no) {
        return byte;
    }

    return byte ^ (1 << (flip_idx % 8));
}
