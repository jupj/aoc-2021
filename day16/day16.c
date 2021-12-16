#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

//#define DEBUG
#include "utils.h"

int part_one(FILE *fp);
unsigned long long part_two(FILE *fp);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fatalf("Please provide input file\n");
    }

    char *infile = argv[1];

    FILE *fp;
    fp = fopen(infile, "r");
    if (fp == NULL) {
        fatalf("Cannot open %s: %s\n", infile, strerror(errno));
    }

    printf("Part one: %d\n", part_one(fp));

    if (fseek(fp, 0, SEEK_SET)) {
        efatal("fseek failed");
    }

    printf("Part two: %lld\n", part_two(fp));

    fclose(fp);
    return 0;
}

/*** Main parts ***/

typedef unsigned long long value;

typedef struct {
    // 64-bits for bit buffer
    value bits;
    // len tracks number of unparsed bits
    int len;
    // n tracks the total number of bits read
    int n;

    FILE *stream;
} bit_reader;

bit_reader br_new(FILE *stream) {
    bit_reader r = {
        .bits = 0,
        .len = 0,
        .n = 0,
        .stream = stream,
    };
    return r;
}

// br_read reads n bits as a value
value br_read(bit_reader *r, int n) {
    while (r->len < n) {
        // Read more bits into r->bits
        int c = fgetc(r->stream);
        switch (c) {
        case EOF:
            fatalf("EOF\n");
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            r->bits = (r->bits << 4) + (c - '0');
            r->len += 4;
            break;
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            r->bits = (r->bits << 4) + 10 + (c - 'A');
            r->len += 4;
            break;
        default:
            fatalf("non-hexadecimal: %c\n", c);
        }
    }

    // Read the value from r->bits
    value mask = (1 << n) - 1;
    value val = (r->bits >> (r->len - n)) & mask;

    // Update counters
    r->len -= n;
    r->n += n;

    return val;
}

typedef struct {
    int version, typeID;
} header;

typedef struct {
    header header;
} packet;

typedef void (*callback)(void *state, header *h);
typedef value (*operator)(value acc, value val);

value op_sum(value acc, value val) { return acc + val; }
value op_product(value acc, value val) { return acc * val; }
value op_min(value acc, value val) {
    if (val < acc) {
        return val;
    }
    return acc;
}
value op_max(value acc, value val) {
    if (val > acc) {
        return val;
    }
    return acc;
}
value op_gt(value a, value b) { return a > b; }
value op_lt(value a, value b) { return a < b; }
value op_eq(value a, value b) { return a == b; }

operator optab[] = {
    op_sum,
    op_product,
    op_min,
    op_max,
    NULL,
    op_gt,
    op_lt,
    op_eq
};
#define N_OPTAB (sizeof(optab) / sizeof(operator))

value parse_packet(bit_reader *r, void *state, callback cb);

value parse_literal_value(bit_reader *r) {
    const int mask_more_groups = 0x10;
    const int mask_value = 0x0F;

    value val = 0;
    int group = mask_more_groups;
    int cnt = 0;
    while (group & mask_more_groups) {
        val <<= 4;
        group = br_read(r, 5);
        val += group & mask_value;
        cnt++;
    }
    return val;
}

value parse_subpkt_bits(bit_reader *r, operator op, void *state, callback cb) {
    // Read payload length in bits
    int len = br_read(r, 15);
    int endN = r->n + len;

    if (len == 0) {
        return 0;
    }

    value acc = parse_packet(r, state, cb);
    while (r->n < endN) { 
        acc = op(acc, parse_packet(r, state, cb));
    }
    return acc;
}

value parse_subpkt_cnt(bit_reader *r, operator op, void *state, callback cb) {
    // Read payload length as number packets
    int cnt = br_read(r, 11);
    if (cnt == 0) {
        return 0;
    }

    value acc = parse_packet(r, state, cb);
    for (int i=1; i<cnt; i++) {
        acc = op(acc, parse_packet(r, state, cb));
    }
    return acc;
}

// parse_packet parses a packet, and calls cb with this packet
// Returns the number of bits read
value parse_packet(bit_reader *r, void *state, callback cb) {
    packet pkt;
    // Read header
    pkt.header.version = br_read(r, 3);
    pkt.header.typeID = br_read(r, 3);

    if (cb != NULL) {
        cb(state, &pkt.header);
    }

    if (pkt.header.typeID > N_OPTAB) {
        fatalf("Unsupported type ID %d\n", pkt.header.typeID);
    }

    if (pkt.header.typeID == 4) {
        return parse_literal_value(r);
    } else {
        operator op = optab[pkt.header.typeID];
        int lenTypeID = br_read(r, 1);
        if (lenTypeID == 0) {
            return parse_subpkt_bits(r, op, state, cb);
        }
        return parse_subpkt_cnt(r, op, state, cb);
    }
}

void cb_sum_versions(int *sum, header *h) {
    *sum += h->version;
}

int part_one(FILE *fp) {
    bit_reader r = br_new(fp);

    int versionsum = 0;
    parse_packet(&r, &versionsum, (callback) cb_sum_versions);

    return versionsum;
}

unsigned long long part_two(FILE *fp) {
    // Fail: answer 179914287 is too low
    bit_reader r = br_new(fp);
    return parse_packet(&r, NULL, NULL);
}
