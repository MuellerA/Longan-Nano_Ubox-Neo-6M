<h1>Longan Nano with u-blox NEO-6M</h1>

<p>Connect a u-blox NEO-6M module with a Longan Nano and display UTC time and position data</p>

<img src="img.jpg" alt="" width="500px"/>

<h2>Usage</h2>
<p>Short button press:</p>
<p>Long button press:</p>

<h2>Log File Format</h2>
<p>The logfile format is compatible with <a href="http://www.gpsbabel.org/index.html">gpsbabel</a>'s unicsv format. Gpsbabel can convert the logfile to many different formats (try "gpsbabel -h" for a full list of formats). E.g. to create a kml file for import in Google Earth use the command:</p>
<pre>gpsbabel -i unicsv -f 'yyymmdd hhmmss.csv' -o kml -F 'yyymmdd hhmmss.kml'</pre>
