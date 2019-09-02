using System;

namespace MWR.C3.WebController.RandomExtentions
{
    public static class RandomExtentions
    {
        public static ushort NextU16(this Random self)
        {
            var buffer = new byte[sizeof(ushort)];
            self.NextBytes(buffer);
            return BitConverter.ToUInt16(buffer);
        }

        public static uint NextU32(this Random self)
        {
            var buffer = new byte[sizeof(uint)];
            self.NextBytes(buffer);
            return BitConverter.ToUInt32(buffer);
        }

        public static ulong NextU64(this Random self)
        {
            var buffer = new byte[sizeof(ulong)];
            self.NextBytes(buffer);
            return BitConverter.ToUInt64(buffer);
        }
    }
}
