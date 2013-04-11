local ffi = require "ffi"

local lib = ffi.load("build/libilnetwork.so", true)

ffi.cdef [[

struct timeval {
    int tv_sec;
    int tv_usec;
};

typedef struct ilN_packet {
    struct timeval arrival;
    uint16_t seq;
    uint16_t ack;
    uint8_t flags;
    uint8_t channel;
    uint32_t object;
    uint32_t type;
    uint16_t length;
    unsigned char *data;
} ilN_packet;

unsigned char *ilN_packet_serialize(const ilN_packet* self, size_t *size);
ilN_packet *ilN_packet_deserialize(const unsigned char *data, size_t size);

]]

function print_packet(p)
    s = string.format("<seq:%u> <ack:%u> <flags:%x> <channel:%u>", p.seq, p.ack, p.flags, p.channel)
    if p.object > 0 then
        s = s .. string.format(" <object:%u>", p.object)
    elseif p.type > 0 then
        s = s .. string.format(" <type:%u>", p.type)
    end
    s = s.." <data:"
    for i=1, p.length do
        s = s .. string.format("%x", p.data[i])
    end
    s = s..">"
    print(s)
end

if arg[1] == "-d" then
    local packet = lib.ilN_packet_deserialize(arg[2], #arg[2])
    print_packet(packet)
elseif arg[1] == "-f" then
    local file = io.open(arg[2], 'r')
    local contents = ""
    for l in file:lines() do
        for h in l:gsub("#.*$", ""):gmatch("%x%x") do
            --print(h)
            contents = contents..string.char(tonumber(h,16))
        end
    end
    local packet = lib.ilN_packet_deserialize(contents, #contents)
    print_packet(packet)
elseif arg[1] == "-e" then
    -- TODO: packet serializing
end

