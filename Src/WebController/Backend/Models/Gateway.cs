using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace MWR.C3.WebController.Models
{
    public class Gateway : Agent
    {
        public ICollection<Relay> Relays { get; set; }

        public ICollection<Connector> Connectors { get; set; }
    }
}
