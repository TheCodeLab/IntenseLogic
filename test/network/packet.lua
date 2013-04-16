local ffi = require "ffi"

local lib = ffi.load("build/libilnetwork.so", true)

ffi.cdef [[

struct timeval {
    long tv_sec;
    long tv_usec;
};

typedef struct ilN_packet {
    struct timeval arrival;
    uint8_t flags;
    uint8_t channel;
    uint16_t event;
    uint32_t object;
    uint32_t type;
    uint16_t length;
    unsigned char *data;
} ilN_packet;

unsigned char *ilN_packet_serialize(const ilN_packet* self, size_t *size);
ilN_packet *ilN_packet_deserialize(const unsigned char *data, size_t size);

]]

function print_packet(p)
    s = string.format("<flags:%x> <channel:%u>", p.seq, p.ack, p.flags, p.channel)
    if p.object > 0 then
        s = s .. string.format(" <object:%u>", p.object)
    elseif p.type > 0 then
        s = s .. string.format(" <type:%u>", p.type)
    end
    s = s.." <data:"
    for i=0, p.length-1 do
        --s = s .. string.format("%x", p.data[i])
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
    assert(#arg == 7, "Usage: "..arg[0].." flags channel event object type [data]")
    local n = ffi.new("size_t[1]")
    local p = ffi.new("ilN_packet")
    p.flags = tonumber(arg[2])
    p.channel = tonumber(arg[3])
    p.event = tonumber(arg[4])
    p.object = tonumber(arg[5])
    p.type = tonumber(arg[6])
    p.data = ffi.cast("unsigned char*",arg[7])
    p.length = #arg[7]
    local res = lib.ilN_packet_serialize(ffi.cast("ilN_packet*",p), ffi.cast("size_t*",n))
    local s = ""
    local i = 0
    print("length: "..tostring(n[0]))
    while i<n[0] do
        s = s..string.format(" %x", res[i])
        i = i+1
    end
    print(s)
end

