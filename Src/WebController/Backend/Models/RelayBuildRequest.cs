using MWR.C3.WebController.Comms;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using static MWR.C3.WebController.Models.Build;

namespace MWR.C3.WebController.Models
{
    public class RelayBuildRequest
    {
        [JsonConverter(typeof(StringEnumConverter))]
        public BinaryType Type { get; set; }

        [JsonConverter(typeof(StringEnumConverter))]
        public Architecture Architecture { get; set; }

        [JsonConverter(typeof(HexStringJsonConverter))]
        public ulong ParentGatewayBuildId { get; set; }

        public string Name { get; set; }

        public JArray StartupCommands { get; set; }
    }
}
