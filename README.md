# multi-Pings

A tool for collecting packet delay and loss statistics using ICMP echo.

## Description
Multi-Pings is a tool for collecting packet delay and loss statistics using ICMP echo. It is not a linux command, such as ping or mping.<br>
* Ability to send ICMP ECHO_REQUEST to multiple hosts simultaneously
* Host ips be read from stdin
* The results are output in a specified format

## Usage
#### How to use
* make
* Usage : ./pings -o [OUTPUT_STYLE] <br>
  OUTPUT_STYLE := { stream | json | yaml }

#### Example
* In directory script, a file called pings.py written by PYTHON<br>
  import this module and use like this:<br>
<pre><code>
    p = Pings( hosts, format = fmt, logger = logger )<br>
    rst = p.run()
</pre></code>
* For more detailed information of Pings, please refer to [pings.py](script/pings.py)
