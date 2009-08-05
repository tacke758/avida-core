#!C:\python\python

"""
// ----------------------------------------------------------------------------
// NAME    : socktools.py [IMPLEMENTATION]                                         
// PURPOSE : Basic wrappers for the berkely API meant to simplify various 
//           need for use with sockets.  Currently we just wrap the send()
//           and recv() routines to ensure delivery of complete payloads.                                  
//                                                                              
// FILE     : $Source: /cvsroot/fit140/socktools.py,v $                          
// REVISION : $Revision: 1.1.1.1 $                                                  
// AUTHOR   : $Author: dtb26 $                                                  
// DATE     : $Date: 2009/04/15 14:14:36 $                                      
// TAG NAME : $Name:  $                                                         
// LOCKER   : $Locker:  $                                                       
// STATE    : $State: Exp $                                                     
// ----------------------------------------------------------------------------
""" 

import socket
import binascii
from ctypes import create_string_buffer


"""
// -------------------------------------------------------------------
// NAME    : toSocket()                                          
// PURPOSE : Transmits the contents of a supplied buffer over an
//           established (connected) TCP stream socket.      
// ARGS    : fd    - socket to transmit over
//           bytes - the size of the payload to be transmitted
//           txbuffer - the actual data to transmit
// RETURNS : none                                            
// NOTES   : Python < 3.x cannot transmit anything other than strings
// --------------------------------------------------------------------
"""      
def toSocket( fd, bytes, txbuffer ):
	sent_bytes = 0
	remaining_bytes = bytes
	delivered_bytes = 0
	
	while remaining_bytes > 0:
		sent_bytes = fd.send( txbuffer )
		if sent_bytes == 0:
			# raise an error
			raise RuntimeError, "In toSocket(): socket connection broken (0 bytes transmitted)"
		else:
			remaining_bytes = remaining_bytes - sent_bytes
			delivered_bytes = delivered_bytes + sent_bytes
			sent_bytes = 0
	
	return delivered_bytes
# eof toSocket()


"""
// -------------------------------------------------------------------
// NAME    : receive()
// PURPOSE : retrieves data from a read-ready socket.  The routine 
//           reads (or blocks) until it has read buffer_sz bytes or
//           until the terminator character is encountered.
// ARGS    : fd         - the socket to recv from
//           buffer_sz  - max size of data load to retreive
//           terminator - code to signify end-of-transmission
// RETURNS : string - data received
// NOTES   : none
// --------------------------------------------------------------------
""" 
def receive( fd, buffer_sz, terminator = '\0' ):
	rxbuffer = ''
	while len(rxbuffer) < buffer_sz:
		chunk = fd.recv(buffer_sz - len(rxbuffer))
		if chunk == '':
			raise RuntimeError, "socket connection broken"
		rxbuffer = rxbuffer + chunk
		
		sz = len(chunk) - 1
		if chunk[sz] == terminator:
			break;

	return rxbuffer
#eof receive()


"""
// -------------------------------------------------------------------
// NAME    : hexToString()
// PURPOSE : A debug routine which transforms a string into its equivalent
//           hexadecimal notation (how it was transmitted).
// ARGS    : string - data to convert
// RETURNS : string - the hexified string
// NOTES   : none
// --------------------------------------------------------------------
""" 
def hexToString( buffer ):
	final = ""
	char = ''
	data = buffer
	for i in range(0,len(data)):
		char = data[i]
		char = binascii.hexlify(char)
		if len(final) > 0:
			final = final + " "
		final = final + char
		
	return final
# eof hexToString()


"""
VERSION LOG                                    
===========                                    
$Log: socktools.py,v $
Revision 1.1.1.1  2009/04/15 14:14:36  dtb26
initial importation of v1 code

"""