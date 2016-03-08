#!/usr/bin/env python2.6
# coding: utf-8

import os
import socket
import logging
import subprocess

import json
import yaml
try:
    from yaml import CLoader as Loader, CDumper as Dumper
except ImportError:
    from yaml import Loader, Dumper


dumbLogger = logging.Logger( '_dumb_' )
dumbLogger.setLevel( logging.CRITICAL )


class PingsError( Exception ): pass
class HostError( PingsError ): pass
class RunError( PingsError ): pass
class OutputError( PingsError ): pass

class Pings( object ):

    def __init__( self, destinations, bin = None, cwd = None,
                            format = 'json', logger=dumbLogger ):

        self.bin = bin or './pings'
        self.cwd = cwd or os.getcwd()
        self.logger = logger

        self.format = format
        try:
            self.make_result = getattr( self, 'make_result_' + self.format )
        except AttributeError:
            raise PingsError( 'pings not supported format: ' + self.format )

        ips = []
        for x in destinations:
            try:
                ips.append( to_ip( x ) )
            except socket.gaierror as e:
                raise HostError( repr(e) + ' while get ip from: ' + x )

        self.hostip = dict( zip( destinations, ips ) )
        self.dests = list(set(self.hostip.values()))

    def run( self ):

        p = subprocess.Popen(
                 [self.bin, '-o', self.format],
                 cwd = self.cwd,
                 close_fds = True,
                 stdin = subprocess.PIPE,
                 stdout = subprocess.PIPE,
                 stderr = subprocess.PIPE, )

        p.stdin.write('\n'.join(self.dests) + '\n')

        out, err = p.communicate()
        p.wait()

        code = p.returncode
        if code != 0:
            raise RunError( 'run pings: ' + repr( (code, out, err) ) )

        r = self.make_result( out )

        rst = {}
        for host, ip in self.hostip.items():

            if ip not in r:
                raise OutputError( 'invalid output lack of ip: ' + ip )

            rst[ host ] = r[ ip ]
            rst[ host ][ 'dest' ] = host

        return rst

    def _parse_stream_line( self, line ):

        # ip:%s dest:%s sent:%d recv:%d loss:%d avg:%d

        d = {}
        for item in line.split( ' ' ):
            k, v = item.split( ':', 1 )

            if v:
                if k not in ( 'ip', 'dest' ):
                    v = int( float( v ) )
            else:
                v = None

            d[ k ] = v

        return d

    def make_result_stream( self, out ):

        r = {}
        for line in out.split('\n'):
            line = line.strip()
            if not line:
                continue

            try:
                d = self._parse_stream_line( line )
            except Exception:
                raise OutputError( 'invalid line: ' + line )

            r[ d[ 'ip' ] ] = d

        return r

    def make_result_json( self, out ):

        if out is None:
            return None

        try:
            return json.loads( out, encoding='utf-8' )
        except ValueError:
            raise OutputError( 'invalid json output: ' + out )

    def make_result_yaml( self, out ):

        if out is None:
            return  None

        try:
            return yaml.load( out, Loader )
        except ValueError:
            raise OutputError( 'invalid yaml output: ' + out )


def is_ip4( ip ):

    ip = ip.split( '.' )

    for s in ip:
        if s == '':
            continue

        if not s.isdigit():
            return False

        i = int( s )
        if i<0 or i>255:
            return False

    return len( ip ) == 4

def to_ip(addr):

    if is_ip4(addr):
        return addr

    return socket.gethostbyname(addr)

if __name__ == '__main__':
    import sys
    import pprint

    logger = logging.Logger( 'xx' )
    logger.addHandler( logging.StreamHandler( sys.stdout ) )
    logger.setLevel( logging.INFO )

    hosts = [
            'google.com',
            'baidu.com',
            'sina.com',
    ]

    for fmt in ( 'json', 'yaml', 'stream' ):
        print '------', fmt, ' format: '
        p = Pings( hosts, format = fmt, logger = logger )
        rst = p.run()

        pprint.pprint( rst )
