#!/usr/bin/env python3
"""
Convert a Saleae trace CSV into frames.

GENERATED WITH AI ASSISTANCE
"""

import csv
import sys
from pathlib import Path

def parse_hex_token(tok):
    tok = tok.strip()
    if tok.startswith('0x') or tok.startswith('0X'):
        tok = tok[2:]
    if tok == '':
        return None
    try:
        return int(tok, 16) & 0xFF
    except ValueError:
        return None


def read_timestamped_bytes(path):
    with open(path, 'r', newline='') as f:
        reader = csv.reader(f)
        for lineno, row in enumerate(reader, 1):
            if not row:
                continue
            # try to find timestamp and data fields
            # many captures: name,type,start_time,duration,data
            if len(row) >= 5:
                ts = row[2].strip()
                data_field = row[-1].strip()
                # data_field may be like 0xAA or 0xAA 0x55 etc.
                toks = data_field.split()
                for tok in toks:
                    b = parse_hex_token(tok)
                    if b is not None:
                        yield (ts, b)
            else:
                # fallback: try to extract any token that looks like hex from the row
                for item in row:
                    b = parse_hex_token(item)
                    if b is not None:
                        # no timestamp available on these rows
                        yield ("0", b)

def frames_from_bytes(iter_bytes):
    """Combine bytes into frames based on AA 55 <len> where len is total bytes in frame starting at AA.
    Yields (timestamp, [byte, ...], valid, reason) where timestamp is the timestamp string of the last byte in the frame,
    valid is True for structurally valid frames with correct checksum, False otherwise; reason is None or a short string
    describing why invalid (e.g., 'skipped', 'incomplete', 'bad_checksum').
    """
    buffer = []  # list of (ts, byte)
    skipped = []  # bytes dropped while resyncing
    for ts, b in iter_bytes:
        buffer.append((ts, b))
        # attempt to extract frames from the front
        while True:
            if len(buffer) < 3:
                break
            # if first two bytes are not AA 55, drop until we find them
            if buffer[0][1] != 0xAA or buffer[1][1] != 0x55:
                # move first byte to skipped and continue
                skipped.append(buffer.pop(0))
                continue
            L = buffer[2][1]
            # basic validation
            if L < 3:
                # invalid length, move first byte to skipped to resync
                skipped.append(buffer.pop(0))
                continue
            if len(buffer) >= L:
                # we have a full frame
                frame_items = buffer[:L]
                # if we have skipped bytes pending, emit them as an invalid frame first
                if skipped:
                    sk_ts = skipped[-1][0]
                    sk_bytes = [b for (_, b) in skipped]
                    yield (sk_ts, sk_bytes, False, 'skipped')
                    skipped = []
                # produce frame timestamp and bytes (use timestamp of last byte)
                frame_ts = frame_items[-1][0]
                frame_bytes = [b for (_, b) in frame_items]

                # checksum validation mimics send_frame logic
                # frame structure: [AA,55,L,CMD, payload..., (payload_checksum?), frame_checksum]
                reason = None
                valid = True
                try:
                    cmd = frame_bytes[3]
                    payload_checksum_present = (cmd == 0xC2 or cmd == 0x22)
                    payload_len = frame_bytes[2] - 5 - (1 if payload_checksum_present else 0)
                    if payload_len < 0:
                        raise ValueError('bad length')
                    payload = frame_bytes[4:4+payload_len] if payload_len > 0 else []
                    idx = 4 + payload_len
                    payload_checksum_byte = None
                    if payload_checksum_present:
                        payload_checksum_byte = frame_bytes[idx]
                        idx += 1
                    frame_checksum_byte = frame_bytes[-1]

                    # compute expected payload checksum when present
                    if payload_checksum_present:
                        payload_sum = sum(payload) & 0xFF
                        if cmd == 0x22 and payload:
                            payload_sum = (payload_sum - payload[0]) & 0xFF
                        if payload_sum != payload_checksum_byte:
                            valid = False
                            reason = 'bad_payload_checksum'

                    # compute expected frame checksum
                    # Note: send_frame computes frame_checksum BEFORE adding payload_checksum,
                    # so the final checksum covers AA..payload (excludes payload_checksum if present).
                    if payload_checksum_present:
                        frame_sum = sum(frame_bytes[:-2]) & 0xFF
                    else:
                        frame_sum = sum(frame_bytes[:-1]) & 0xFF
                    if frame_sum != frame_checksum_byte:
                        valid = False
                        if reason:
                            reason = reason + ';bad_frame_checksum'
                        else:
                            reason = 'bad_frame_checksum'
                except Exception:
                    valid = False
                    if reason:
                        reason = reason + ';parse_error'
                    else:
                        reason = 'parse_error'

                yield (frame_ts, frame_bytes, valid, reason)
                # remove consumed bytes
                buffer = buffer[L:]
                continue
            else:
                # need more bytes
                break
    # end for
    # After processing all input, if there are leftover bytes, treat them as invalid
    # Combine any remaining skipped and buffer bytes
    remaining = skipped + buffer
    if remaining:
        rem_ts = remaining[-1][0]
        rem_bytes = [b for (_, b) in remaining]
        yield (rem_ts, rem_bytes, False, 'incomplete')


def format_frame_line(ts, bytes_list):
    hexs = ' '.join(f"{x:02X}" for x in bytes_list)
def format_frame_line(ts, bytes_list, valid=True, reason=None):
    hexs = ' '.join(f"{x:02X}" for x in bytes_list)
    t = float(ts)
    line = f"{t:06.2f} {hexs}"
    if not valid:
        if reason:
            line = f"{line} [{reason}]"
        else:
            line = f"{line} [invalid frame]"
    return line


def main():
    if len(sys.argv) < 2:
        print("Usage: csv_to_frames.py inputfile [outputfile]")
        sys.exit(2)
    infile = Path(sys.argv[1])
    outfile = None
    if len(sys.argv) >= 3:
        outfile = Path(sys.argv[2])
    frames = frames_from_bytes(read_timestamped_bytes(infile))
    if outfile:
        with open(outfile, 'w') as out:
            for ts, fb, valid, reason in frames:
                out.write(format_frame_line(ts, fb, valid, reason) + "\n")
    else:
        for ts, fb, valid, reason in frames:
            print(format_frame_line(ts, fb, valid, reason))


if __name__ == '__main__':
    main()
