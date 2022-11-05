<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE nta PUBLIC '-//Uppaal Team//DTD Flat System 1.1//EN' 'http://www.it.uu.se/research/group/darts/uppaal/flat-1_2.dtd'>
<nta>
	<declaration>// Place global declarations here.

const int Houses = 1;
typedef int[0,Houses-1] house_t;

const int KEYS = 256;


typedef int[0,KEYS] key_t;
key_t randKey () {
    return fint (random (KEYS));
}
const key_t HouseKeys[Houses] = {1};






meta int key;

const int Owners = 1;
typedef int[0,Owners-1] owner_t;
const house_t House_of_owner[Owners] = {0};

const int Orders = 2;
typedef int[0,Orders-1] order_t;

typedef struct {
    bool to_deliver;
    house_t receipient;
    order_t package;    
    key_t enter_key;
}parcel_t;

meta parcel_t parcel;

broadcast chan unlock_owner[Houses];
broadcast chan unlock_amazon[Houses];
bool locked[Houses];
broadcast chan lock[Houses];
broadcast chan key_from_server[Houses];
broadcast chan start_cam[Houses];
broadcast chan stop_cam[Houses];
broadcast chan order[Houses][Orders];
broadcast chan jam_cam[Houses];

const int Deliverer = 1;
typedef int[0,Deliverer-1] deliver_t;

broadcast chan pickup[Deliverer];</declaration>
	<template>
		<name x="5" y="5">House_T</name>
		<parameter>house_t hid, const int ownerkey, const bool autolock</parameter>
		<declaration>// Place local declarations here.
int usedkey;

int validated_key;

bool validate_key () {
    if (usedkey == 0) {
       return false;    
    }
    else {
        return validated_key == usedkey;
    }
}

clock x;</declaration>
		<location id="id0" x="-442" y="34">
			<label kind="exponentialrate" x="-442" y="51">1000</label>
		</location>
		<location id="id1" x="-110" y="34">
			<committed/>
		</location>
		<location id="id2" x="323" y="136">
		</location>
		<location id="id3" x="323" y="34">
			<committed/>
		</location>
		<location id="id4" x="119" y="34">
		</location>
		<location id="id5" x="-221" y="-153">
			<committed/>
		</location>
		<init ref="id4"/>
		<transition>
			<source ref="id0"/>
			<target ref="id5"/>
			<label kind="guard" x="-399" y="-119">autolock &amp;&amp; x&gt;=2</label>
			<nail x="-417" y="-93"/>
			<nail x="-297" y="-93"/>
			<nail x="-229" y="-93"/>
		</transition>
		<transition>
			<source ref="id2"/>
			<target ref="id2"/>
			<label kind="synchronisation" x="323" y="238">key_from_server[hid]?</label>
			<label kind="assignment" x="323" y="255">validated_key = key</label>
			<nail x="313" y="230"/>
			<nail x="406" y="230"/>
		</transition>
		<transition>
			<source ref="id0"/>
			<target ref="id0"/>
			<label kind="synchronisation" x="-604" y="128">key_from_server[hid]?</label>
			<label kind="assignment" x="-612" y="145">validated_key = key</label>
			<nail x="-519" y="77"/>
			<nail x="-468" y="119"/>
		</transition>
		<transition>
			<source ref="id0"/>
			<target ref="id5"/>
			<label kind="synchronisation" x="-347" y="-170">lock[hid]?</label>
			<nail x="-442" y="-153"/>
		</transition>
		<transition>
			<source ref="id5"/>
			<target ref="id4"/>
			<label kind="synchronisation" x="-178" y="-144">stop_cam[hid]!</label>
			<label kind="assignment" x="-187" y="-119">locked [hid]= true</label>
			<nail x="-221" y="-84"/>
			<nail x="-8" y="-85"/>
		</transition>
		<transition>
			<source ref="id1"/>
			<target ref="id0"/>
			<label kind="guard" x="-340" y="9">validate_key ()</label>
			<label kind="synchronisation" x="-306" y="76">start_cam[hid]!</label>
			<label kind="assignment" x="-306" y="34">locked[hid] = false,
x = 0</label>
			<nail x="-264" y="34"/>
		</transition>
		<transition>
			<source ref="id1"/>
			<target ref="id4"/>
			<label kind="guard" x="-85" y="85">!validate_key ()</label>
			<nail x="-110" y="76"/>
			<nail x="94" y="76"/>
		</transition>
		<transition>
			<source ref="id4"/>
			<target ref="id1"/>
			<label kind="synchronisation" x="-68" y="17">unlock_amazon[hid]?</label>
			<label kind="assignment" x="-51" y="34">usedkey = key</label>
			<nail x="-76" y="34"/>
		</transition>
		<transition>
			<source ref="id4"/>
			<target ref="id4"/>
			<label kind="synchronisation" x="26" y="-118">key_from_server[hid]?</label>
			<label kind="assignment" x="26" y="-93">validated_key = key</label>
			<nail x="77" y="-17"/>
			<nail x="136" y="-17"/>
		</transition>
		<transition>
			<source ref="id2"/>
			<target ref="id4"/>
			<label kind="synchronisation" x="136" y="119">lock[hid]?</label>
			<label kind="assignment" x="137" y="136">locked[hid] = true</label>
			<nail x="119" y="136"/>
		</transition>
		<transition>
			<source ref="id3"/>
			<target ref="id4"/>
			<label kind="guard" x="153" y="51">usedkey != ownerkey</label>
			<nail x="323" y="76"/>
			<nail x="119" y="76"/>
		</transition>
		<transition>
			<source ref="id3"/>
			<target ref="id2"/>
			<label kind="guard" x="332" y="68">usedkey == ownerkey</label>
			<label kind="assignment" x="323" y="85">locked[hid]=false</label>
		</transition>
		<transition>
			<source ref="id4"/>
			<target ref="id3"/>
			<label kind="synchronisation" x="153" y="-76">unlock_owner[hid]?</label>
			<label kind="assignment" x="153" y="-51">usedkey = key</label>
			<nail x="153" y="-25"/>
			<nail x="289" y="-25"/>
		</transition>
	</template>
	<template>
		<name>Attacker</name>
		<declaration>
// Information to attack_defense_tree

owner_t owner;
house_t target;





clock x;
key_t ama_key;
key_t house_key;
key_t pick_key;

parcel_t par;
</declaration>
		<location id="id6" x="-408" y="-272">
			<committed/>
		</location>
		<location id="id7" x="-204" y="-272">
		</location>
		<location id="id8" x="-68" y="-272">
			<committed/>
		</location>
		<location id="id9" x="170" y="-340">
			<label kind="invariant" x="160" y="-323">x&lt;=10</label>
		</location>
		<location id="id10" x="68" y="340">
			<name x="85" y="348">Inside</name>
		</location>
		<location id="id11" x="25" y="93">
			<committed/>
		</location>
		<location id="id12" x="76" y="93">
			<committed/>
		</location>
		<location id="id13" x="204" y="34">
			<label kind="invariant" x="194" y="51">x&lt;=10</label>
		</location>
		<location id="id14" x="-544" y="-102">
			<label kind="invariant" x="-595" y="-136">x&lt;=10</label>
		</location>
		<location id="id15" x="-391" y="42">
			<label kind="invariant" x="-442" y="59">x&lt;=10</label>
		</location>
		<location id="id16" x="0" y="0">
			<urgent/>
		</location>
		<location id="id17" x="0" y="-280">
			<label kind="invariant" x="8" y="-271">x&lt;=5</label>
		</location>
		<location id="id18" x="68" y="238">
			<urgent/>
		</location>
		<init ref="id16"/>
		<transition>
			<source ref="id6"/>
			<target ref="id16"/>
			<label kind="guard" x="-730" y="-382">!par.to_deliver</label>
			<nail x="-408" y="-348"/>
			<nail x="-748" y="-348"/>
			<nail x="-748" y="-170"/>
			<nail x="-238" y="-170"/>
			<nail x="-238" y="-34"/>
		</transition>
		<transition>
			<source ref="id6"/>
			<target ref="id16"/>
			<label kind="guard" x="-688" y="-246">par.to_deliver</label>
			<label kind="assignment" x="-688" y="-221">ama_key = par.enter_key</label>
			<nail x="-476" y="-272"/>
			<nail x="-476" y="-204"/>
			<nail x="-144" y="-204"/>
			<nail x="-136" y="-34"/>
		</transition>
		<transition>
			<source ref="id7"/>
			<target ref="id6"/>
			<label kind="select" x="-390" y="-323">d : deliver_t</label>
			<label kind="synchronisation" x="-365" y="-297">pickup[d]?</label>
			<label kind="assignment" x="-365" y="-272">par = parcel</label>
		</transition>
		<transition>
			<source ref="id8"/>
			<target ref="id7"/>
			<label kind="select" x="-195" y="-314">d : deliver_t</label>
			<label kind="synchronisation" x="-195" y="-297">pickup[d]!</label>
		</transition>
		<transition>
			<source ref="id16"/>
			<target ref="id8"/>
			<nail x="-68" y="0"/>
		</transition>
		<transition>
			<source ref="id18"/>
			<target ref="id16"/>
			<label kind="guard" x="-186" y="204">locked[target]</label>
			<nail x="-204" y="238"/>
			<nail x="-204" y="102"/>
			<nail x="0" y="102"/>
		</transition>
		<transition>
			<source ref="id9"/>
			<target ref="id16"/>
			<label kind="assignment" x="246" y="-263">pick_key = randKey()</label>
			<nail x="229" y="-340"/>
			<nail x="229" y="-178"/>
			<nail x="34" y="-178"/>
		</transition>
		<transition>
			<source ref="id16"/>
			<target ref="id9"/>
			<label kind="assignment" x="43" y="-212">x = 0</label>
			<nail x="25" y="-212"/>
			<nail x="136" y="-212"/>
			<nail x="136" y="-340"/>
		</transition>
		<transition>
			<source ref="id18"/>
			<target ref="id10"/>
			<label kind="guard" x="76" y="272">!locked[target]</label>
		</transition>
		<transition>
			<source ref="id11"/>
			<target ref="id18"/>
			<label kind="synchronisation" x="-161" y="153">unlock_amazon[target]!</label>
			<label kind="assignment" x="-433" y="170">key = ama_key</label>
			<nail x="25" y="127"/>
			<nail x="25" y="204"/>
		</transition>
		<transition>
			<source ref="id16"/>
			<target ref="id11"/>
			<nail x="25" y="68"/>
		</transition>
		<transition>
			<source ref="id12"/>
			<target ref="id18"/>
			<label kind="synchronisation" x="76" y="144">unlock_owner[target]!</label>
			<label kind="assignment" x="76" y="161">key = house_key</label>
			<nail x="76" y="195"/>
		</transition>
		<transition>
			<source ref="id16"/>
			<target ref="id12"/>
			<nail x="76" y="34"/>
		</transition>
		<transition>
			<source ref="id13"/>
			<target ref="id16"/>
			<label kind="assignment" x="154" y="102">house_key = randKey ()</label>
			<nail x="204" y="102"/>
			<nail x="136" y="102"/>
			<nail x="136" y="34"/>
		</transition>
		<transition>
			<source ref="id16"/>
			<target ref="id13"/>
			<label kind="assignment" x="127" y="0">x = 0</label>
			<nail x="136" y="8"/>
		</transition>
		<transition>
			<source ref="id14"/>
			<target ref="id16"/>
			<label kind="assignment" x="-790" y="-85">ama_key = randKey ()</label>
			<nail x="-544" y="0"/>
			<nail x="-17" y="0"/>
		</transition>
		<transition>
			<source ref="id16"/>
			<target ref="id14"/>
			<label kind="assignment" x="-518" y="-127">x = 0</label>
			<nail x="-323" y="0"/>
			<nail x="-323" y="-102"/>
		</transition>
		<transition>
			<source ref="id15"/>
			<target ref="id16"/>
			<label kind="synchronisation" x="-433" y="85">jam_cam[target]!</label>
			<nail x="-391" y="68"/>
			<nail x="0" y="68"/>
		</transition>
		<transition>
			<source ref="id16"/>
			<target ref="id15"/>
			<label kind="assignment" x="-340" y="25">x=0</label>
			<nail x="-391" y="8"/>
		</transition>
		<transition>
			<source ref="id17"/>
			<target ref="id16"/>
			<label kind="select" x="0" y="-254">t : owner_t</label>
			<label kind="assignment" x="0" y="-237">owner = t, target = House_of_owner[t]</label>
		</transition>
		<transition>
			<source ref="id16"/>
			<target ref="id17"/>
			<label kind="assignment" x="-34" y="-140">x = 0</label>
			<nail x="-34" y="0"/>
			<nail x="-34" y="-280"/>
		</transition>
	</template>
	<template>
		<name>Camera</name>
		<parameter>house_t hid, bool Jammable</parameter>
		<declaration>clock unjam;
</declaration>
		<location id="id19" x="102" y="0">
			<name x="92" y="-34">Recording</name>
		</location>
		<location id="id20" x="-136" y="0">
		</location>
		<location id="id21" x="-136" y="-136">
			<name x="-146" y="-170">Jammed</name>
			<label kind="exponentialrate" x="-146" y="-102">1</label>
		</location>
		<location id="id22" x="-306" y="0">
			<committed/>
		</location>
		<branchpoint id="id23" x="-204" y="-68">
		</branchpoint>
		<init ref="id20"/>
		<transition>
			<source ref="id21"/>
			<target ref="id20"/>
			<label kind="guard" x="-76" y="-119">unjam&gt;=10</label>
			<nail x="-76" y="-136"/>
			<nail x="-76" y="-34"/>
			<nail x="-102" y="-34"/>
		</transition>
		<transition>
			<source ref="id23"/>
			<target ref="id20"/>
			<label kind="probability" x="-187" y="-85">10</label>
			<nail x="-136" y="-68"/>
		</transition>
		<transition>
			<source ref="id23"/>
			<target ref="id21"/>
			<label kind="assignment" x="-289" y="-136">unjam = 0</label>
			<label kind="probability" x="-187" y="-161">10</label>
			<nail x="-204" y="-136"/>
		</transition>
		<transition controllable="false">
			<source ref="id22"/>
			<target ref="id23"/>
			<nail x="-306" y="-68"/>
		</transition>
		<transition>
			<source ref="id20"/>
			<target ref="id22"/>
			<label kind="guard" x="-280" y="0">Jammable</label>
			<label kind="synchronisation" x="-280" y="-25">jam_cam[hid]?</label>
		</transition>
		<transition>
			<source ref="id19"/>
			<target ref="id20"/>
			<label kind="synchronisation" x="-118" y="51">stop_cam[hid]?</label>
			<nail x="102" y="68"/>
			<nail x="-136" y="68"/>
		</transition>
		<transition>
			<source ref="id20"/>
			<target ref="id19"/>
			<label kind="synchronisation" x="-93" y="-25">start_cam[hid]?</label>
		</transition>
	</template>
	<template>
		<name>Server_T</name>
		<parameter>key_t pickup_key</parameter>
		<declaration>
const int SIZE = 10;
typedef int[0,SIZE] id_t;
id_t next = 0;
parcel_t packages[SIZE-1];

void enqueue (parcel_t parcel) {
    if (next &lt; SIZE-1) {
        packages[next] = parcel;
        next++;
    }
    
}

parcel_t dequeue () {
    return packages[--next];
}

bool hasParcel () {
    return next == 0;
}


void insert_order(order_t o, house_t h) {
    parcel_t par;
    par.to_deliver = true;
    par.receipient = h;
    par.package = o;
    enqueue (par);
}    

key_t used_key;
deliver_t dd;

void reset_parcel () {
    parcel.to_deliver = false;
    parcel.enter_key = 0;
}</declaration>
		<location id="id24" x="-459" y="0">
			<committed/>
		</location>
		<location id="id25" x="-68" y="0">
		</location>
		<location id="id26" x="510" y="0">
			<committed/>
		</location>
		<location id="id27" x="340" y="0">
			<committed/>
		</location>
		<init ref="id25"/>
		<transition>
			<source ref="id27"/>
			<target ref="id25"/>
			<label kind="synchronisation" x="-50" y="-17">key_from_server[packages[next-1].receipient]!</label>
			<label kind="assignment" x="-50" y="0">key = packages[next-1].enter_key</label>
		</transition>
		<transition>
			<source ref="id24"/>
			<target ref="id25"/>
			<label kind="guard" x="-365" y="34">used_key == pickup_key &amp;&amp; next != 0</label>
			<label kind="synchronisation" x="-441" y="51">pickup[dd]!</label>
			<label kind="assignment" x="-366" y="68">parcel = packages[--next]</label>
			<nail x="-459" y="68"/>
			<nail x="-68" y="68"/>
		</transition>
		<transition>
			<source ref="id24"/>
			<target ref="id25"/>
			<label kind="guard" x="-365" y="-8">used_key != pickup_key || 
next == 0</label>
			<label kind="synchronisation" x="-425" y="-17">pickup[dd]!</label>
		</transition>
		<transition>
			<source ref="id25"/>
			<target ref="id24"/>
			<label kind="select" x="-425" y="-102">d : deliver_t</label>
			<label kind="synchronisation" x="-399" y="-68">pickup[d]?</label>
			<label kind="assignment" x="-341" y="-51">used_key = key,dd =d</label>
			<nail x="-76" y="-8"/>
			<nail x="-102" y="-51"/>
			<nail x="-459" y="-51"/>
		</transition>
		<transition>
			<source ref="id26"/>
			<target ref="id27"/>
			<label kind="assignment" x="391" y="17">packages[next-1].enter_key = randKey()</label>
		</transition>
		<transition>
			<source ref="id25"/>
			<target ref="id26"/>
			<label kind="select" x="-51" y="-161">o :order_t,
h : house_t</label>
			<label kind="synchronisation" x="-51" y="-110">order[h][o]?</label>
			<label kind="assignment" x="-51" y="-127">insert_order(o,h)</label>
			<nail x="-68" y="-68"/>
			<nail x="510" y="-68"/>
		</transition>
	</template>
	<template>
		<name>Owner_t</name>
		<parameter>const house_t hid, key_t owner_key</parameter>
		<declaration>clock x;
</declaration>
		<location id="id28" x="-102" y="0">
			<name x="-93" y="-25">Home</name>
			<label kind="invariant" x="-51" y="0">x&lt;=24</label>
			<label kind="exponentialrate" x="-112" y="34">1</label>
		</location>
		<location id="id29" x="-306" y="0">
			<label kind="invariant" x="-316" y="17">x&lt;=8</label>
		</location>
		<location id="id30" x="-102" y="-85">
			<label kind="exponentialrate" x="-85" y="-85">1000</label>
		</location>
		<init ref="id29"/>
		<transition>
			<source ref="id30"/>
			<target ref="id29"/>
			<label kind="guard" x="-356" y="-204">locked[hid]</label>
			<nail x="-102" y="-170"/>
			<nail x="-374" y="-170"/>
			<nail x="-374" y="0"/>
		</transition>
		<transition>
			<source ref="id30"/>
			<target ref="id28"/>
			<label kind="guard" x="-102" y="-51">!locked[hid]</label>
		</transition>
		<transition>
			<source ref="id28"/>
			<target ref="id28"/>
			<label kind="select" x="-119" y="68">o : order_t</label>
			<label kind="synchronisation" x="-119" y="85">order[hid][o]!</label>
			<nail x="-127" y="68"/>
			<nail x="-42" y="68"/>
		</transition>
		<transition>
			<source ref="id28"/>
			<target ref="id29"/>
			<label kind="guard" x="-229" y="17">x&gt;=24</label>
			<label kind="synchronisation" x="-263" y="-17">lock[hid]!</label>
			<label kind="assignment" x="-255" y="0">x:=0</label>
		</transition>
		<transition>
			<source ref="id29"/>
			<target ref="id30"/>
			<label kind="synchronisation" x="-289" y="-102">unlock_owner[hid]!</label>
			<label kind="assignment" x="-289" y="-85">key = owner_key</label>
			<nail x="-307" y="-85"/>
		</transition>
	</template>
	<template>
		<name>Deliver_T</name>
		<parameter>deliver_t did, key_t k</parameter>
		<declaration>clock x;
clock deliver_clock;

parcel_t deliver;</declaration>
		<location id="id31" x="-374" y="-527">
			<label kind="exponentialrate" x="-384" y="-493">1000</label>
		</location>
		<location id="id32" x="-705" y="-527">
			<label kind="exponentialrate" x="-731" y="-518">1</label>
		</location>
		<location id="id33" x="-884" y="-527">
			<committed/>
		</location>
		<location id="id34" x="-1054" y="-527">
		</location>
		<location id="id35" x="-1224" y="-527">
			<label kind="invariant" x="-1283" y="-519">x&lt;=1</label>
		</location>
		<location id="id36" x="-374" y="-663">
			<label kind="invariant" x="-365" y="-722">deliver_clock &lt;= 1</label>
		</location>
		<location id="id37" x="-782" y="-527">
			<label kind="invariant" x="-799" y="-510">x&lt;=10</label>
		</location>
		<location id="id38" x="-1054" y="-663">
			<committed/>
		</location>
		<location id="id39" x="-1054" y="-765">
			<committed/>
		</location>
		<branchpoint id="id40" x="-884" y="-663">
		</branchpoint>
		<init ref="id35"/>
		<transition>
			<source ref="id39"/>
			<target ref="id35"/>
			<nail x="-1292" y="-765"/>
			<nail x="-1292" y="-527"/>
		</transition>
		<transition>
			<source ref="id40"/>
			<target ref="id39"/>
			<label kind="probability" x="-1036" y="-748">0</label>
			<nail x="-884" y="-765"/>
		</transition>
		<transition>
			<source ref="id40"/>
			<target ref="id38"/>
			<label kind="probability" x="-1036" y="-646">100</label>
		</transition>
		<transition controllable="false">
			<source ref="id36"/>
			<target ref="id40"/>
			<label kind="guard" x="-892" y="-629">deliver_clock &gt;= 1</label>
			<label kind="assignment" x="-731" y="-654">deliver.to_deliver = false,
deliver.enter_key = 0,
x=0</label>
		</transition>
		<transition>
			<source ref="id37"/>
			<target ref="id32"/>
			<label kind="guard" x="-765" y="-552">x&gt;=3</label>
		</transition>
		<transition>
			<source ref="id38"/>
			<target ref="id35"/>
			<label kind="synchronisation" x="-1207" y="-620">lock[deliver.receipient]!</label>
			<nail x="-1224" y="-663"/>
		</transition>
		<transition>
			<source ref="id31"/>
			<target ref="id32"/>
			<label kind="guard" x="-654" y="-408">locked[deliver.receipient]</label>
			<nail x="-374" y="-417"/>
			<nail x="-705" y="-417"/>
		</transition>
		<transition>
			<source ref="id31"/>
			<target ref="id36"/>
			<label kind="guard" x="-374" y="-569">!locked[deliver.receipient]</label>
			<label kind="assignment" x="-348" y="-612">deliver_clock = 0</label>
		</transition>
		<transition>
			<source ref="id32"/>
			<target ref="id31"/>
			<label kind="synchronisation" x="-697" y="-519">unlock_amazon[deliver.receipient]!</label>
			<label kind="assignment" x="-697" y="-502">key = deliver.enter_key</label>
		</transition>
		<transition>
			<source ref="id33"/>
			<target ref="id37"/>
			<label kind="guard" x="-918" y="-561">deliver.to_deliver</label>
			<label kind="assignment" x="-858" y="-527">x = 0</label>
		</transition>
		<transition>
			<source ref="id33"/>
			<target ref="id35"/>
			<label kind="guard" x="-1173" y="-459">!deliver.to_deliver</label>
			<label kind="assignment" x="-1206" y="-459">x=0</label>
			<nail x="-884" y="-459"/>
			<nail x="-1224" y="-459"/>
		</transition>
		<transition>
			<source ref="id34"/>
			<target ref="id33"/>
			<label kind="synchronisation" x="-1028" y="-552">pickup[did]?</label>
			<label kind="assignment" x="-1036" y="-527">deliver = parcel</label>
		</transition>
		<transition>
			<source ref="id35"/>
			<target ref="id34"/>
			<label kind="synchronisation" x="-1190" y="-552">pickup[did]!</label>
			<label kind="assignment" x="-1190" y="-527">key = k</label>
		</transition>
	</template>
	<system>// Place template instantiations here.

const bool JamPrevention = false;
const bool auto_lock[Houses] = {true};

House (const house_t h) = House_T (h,HouseKeys[h],auto_lock[h]);
Owner (const owner_t ot) = Owner_t (House_of_owner[ot],HouseKeys[ot]);
Server = Server_T ( 1);
Deliver (const deliver_t d) =  Deliver_T (d, 1);
Cam (const house_t  t) = Camera (t,!JamPrevention);

// List one or more processes to be composed into a system
system House, Owner,Server,Deliver,Cam,Attacker;
    </system>
	<queries>
		<query>
			<formula></formula>
			<comment></comment>
		</query>
		<query>
			<formula></formula>
			<comment></comment>
		</query>
		<query>
			<formula></formula>
			<comment></comment>
		</query>
		<query>
			<formula>Pr[&lt;=100] (&lt;&gt; Attacker.Inside &amp;&amp; !Owner(Attacker.owner).Home &amp;&amp; Cam(Attacker.target).Jammed)</formula>
			<comment></comment>
		</query>
	</queries>
</nta>
