/*
 * Copyright (C) 2015 SDN-WISE
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package org.contikios.cooja.sdnwise;;

import com.github.sdnwiselab.sdnwise.mote.battery.Battery;
import com.github.sdnwiselab.sdnwise.mote.core.*;
import com.github.sdnwiselab.sdnwise.util.NodeAddress;
import org.contikios.cooja.*;

/**
 * @author Sebastiano Milardo
 */
public class CoojaMote extends AbstractCoojaMote {

    public CoojaMote() {
        super();
    }

    public CoojaMote(MoteType moteType, Simulation simulation) {
        super(moteType, simulation);
    }

    @Override
    public final void init() {
        battery = new Battery();
        core = new MoteCore((byte) 1, new NodeAddress(this.getID()), battery);
        core.start();
        startThreads();
    }
}
