#!C:\python\python
"""
// ----------------------------------------------------------------------------
// NAME    : client.py [IMPLEMENTATION]                                         
// PURPOSE : Commandline client for testing the Avida-Ed communications
//           protocol.  This tool allows you to transmit commands and view
//           server responses.  Communications are displayed raw and in hex
//           format.                                        
//                                                                              
// FILE     : $Source: /cvsroot/fit140/client.py,v $                          
// REVISION : $Revision: 1.1.1.1 $                                                  
// AUTHOR   : $Author: dtb26 $                                                  
// DATE     : $Date: 2009/04/15 14:14:36 $                                      
// TAG NAME : $Name:  $                                                         
// LOCKER   : $Locker:  $                                                       
// STATE    : $State: Exp $                                                     
// ----------------------------------------------------------------------------
""" 

import socket
import select
import socktools
import os
import sys
import stat
import avidacomm


# ---------- Symbolics
APPNAME = "Avida-Ed Client"
APPVER = "v0.2"

bufsize = 4096
host = 'localhost'
port = 6712
rxbuffer = ""
connected = 0
running = 1



# ----------- Prototypes

"""
// -------------------------------------------------------------------
// NAME    : doMenu()
// PURPOSE : A simple routine to display a menu to stdout where it will
//           block until a keystroke is captured.
// ARGS    : none
// RETURNS : string - keystroke
// NOTES   : none
// --------------------------------------------------------------------
"""
def doMenu():
	response = ""
	menu = "\nSelect Command\n"
	menu = menu + "===============\n"
	menu = menu + "S[e]nd request\n"
	menu = menu + "[J]oin request\n"
	menu = menu + "[R]emove request\n"
	menu = menu + "[S]ubmit request\n"
	menu = menu + "[U]pdate request\n"
	menu = menu + "[Q]uit\n"
	menu = menu + "===============\n"
	menu = menu + "$"
	
	while len(response) == 0:
		response = raw_input(menu)
		
	return response
#eof doMenu()


"""
// -------------------------------------------------------------------
// NAME    : cmdHandler()
// PURPOSE : Builds and transmits the Avida-Ed command & control
//           structures... essentially fire & forget.
// ARGS    : cmd - ascii uppercase char
// RETURNS : int - avida-ed protocol-defined error code
// NOTES   : Python has no case-select structure, thus we're forced
//           to use the somewhat slower if-elif format.
// --------------------------------------------------------------------
"""
def cmdHandler( cmd ):
	
	status = avidacomm.ERR_NO_ERROR
	
	if line == "E":
		fstats = os.stat ( 'test.file' )
		
		print "[SYS]: issuing command CMD_SEND_REQUEST"
		print "[CMD]: file name = " + "test.file"
		print "[CMD]: file size = " + str(fstats[stat.ST_SIZE])
		print "[CMD]: trans type = TYPE_FILE_BINARY"
		
		f = open("test.file", "rb")
		status = avidacomm.SendRequest( fd, "test.file", int(fstats[stat.ST_SIZE]), f, avidacomm.TYPE_FILE_BINARY )
		
	elif line == "J":
		print "[SYS]: issuing command CMD_JOIN_REQUEST"
		print "[CMD]: software-id = " + avidacomm.AVIDA_SOFTWARE_ID
		print "[CMD]: software-ver = " + avidacomm.AVIDA_SOFTWARE_VER
		status = avidacomm.JoinRequest(fd)

	elif line == "R":
		print "[SYS]: issuing command CMD_REMOVE_REQUEST"
		print "[CMD]: client-id = bobsdish"
		print "[CMD]: dish position = 5"
		status = avidacomm.RemoveRequest( fd, "bobsdish", 5 )

	elif line == "S":
		print "[SYS]: issuing command CMD_SUBMIT_REQUEST"
		print "[CMD]: client-id = bobsotherdish"
		print "[CMD]: dish position = 12"
		status = avidacomm.SubmitRequest( fd, "bobsotherdish", 12 )

	elif line == "U":
		print "[SYS]: issuing command CMD_UPDATE_REQUEST"
		print "[CMD]: type = TYPE_UPDATE_FULL"
		status = avidacomm.UpdateRequest( fd )

	else:
		errstr = "[ERR] " + line + " is not a recognized command!"
		print errstr
		doMenu()
	
	return status
	
#eof cmdHandler()


"""
// -------------------------------------------------------------------
// NAME    : errHandler()
// PURPOSE : Wrapper to handle error displays which would otherwise
//           result in a lot of redundant code.  
// ARGS    : cmd - ascii uppercase char
// RETURNS : int - avida-ed protocol-defined error code
// NOTES   : In C/C++ this should probably be inlined; I'm not sure
//           what Python's capabilities are in this regard and since
//           the language has the nasty habit of repurposing otherwise
//           standard terminology, its not an albatros I wish to waste
//           cycles on at this time.
// --------------------------------------------------------------------
"""
def errHandler( avida_error, stay_connected, stay_running ):
	
	print avidacomm.errorToString( avida_error )
	connected = stay_connected
	running = stay_running
	
#eof errHandler()





# ----------- Driver

# the heartbeat is fairly straight forward; so long as the system is running
# we'll attempt to connect a socket.  So long as we stay connected, the
# system will run the menu, grab input, execute an appropriate avida-ed 
# command and finally look for a response... wash, rinse, & repeat until
# connection broken.  If you want the system to die, turn off the running
# flag, otherwise the system will attempt to reconnect.

print "\n\nRunning " + APPNAME + " " + APPVER + " ..."

while running:
	
	# if we're not connected, do so or die
	if not connected:
		try:
			fd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			fd.connect((host,port))
			connected = 1;
		except socket.error, (value,message): 
			if fd: 
				fd.close() 
				print "[ERR] while opening socket: " + message 
				running = 0
				sys.exit(1)
	
	
	inputs = [fd]
	outputs = [fd]
	excepts = []
	while connected:
		
			# check for keyboard input
			line = doMenu().upper()
			if line == "Q":
				print "\nShutting down... G'bye!\n"
				running = 0
				connected = 0
			else:
				result = cmdHandler( line.upper() )
				if result != avidacomm.ERR_NO_ERROR: errHandler( result, 0, 0 )
			
			if connected:
				inputready,outputready,exceptready = select.select(inputs,[],[])
				for s in inputready:
					if s == fd:
						# handle the one and only client socket
						rxbuffer = socktools.receive( fd, bufsize, avidacomm.ETB )
						parse = 1
						
						if rxbuffer > 0:
							print "[SYS]: response received [data:" + rxbuffer + "]"
							print "[DBG]: response received [data:" + socktools.hexToString(rxbuffer) + "]"
							cmd = avidacomm.getCommand( rxbuffer )
							if cmd == avidacomm.CMD_JOIN_RSP:
								print "[SYS]: response from server CMD_JOIN_RSP"
							elif cmd == avidacomm.CMD_SUBMIT_RSP:
								print "[SYS]: response from server CMD_SUBMIT_RSP"
							elif cmd == avidacomm.CMD_UPDATE_RSP:
								print "[SYS]: response from server CMD_UPDATE_RSP"
							elif cmd == avidacomm.CMD_SEND_RSP:
								print "[SYS]: response from server CMD_SEND_RSP"
							elif cmd == avidacomm.CMD_REMOVE_RSP:
								print "[SYS]: response from server CMD_REMOVE_RSP"
							else:
								print "[ERR]: unknown response was received ??"
								parse = 0
							
							if parse:
								payload = avidacomm.getPayload( rxbuffer )
								print "[DBG]: payload is  [" + payload + "]  (bytes:" + str(len(payload)) + ")"
								result = ord(payload[0])
								reason = ord(payload[1])
								print "[CMD]: result = " + str(result) + " (ACK:0x06 NAK:0x07)"
								print "[CMD]: reason = " + str(reason)
								print "[CMD]: bytes rcvd by server = " + str(len(payload))
						else:
							print "[ERR]: An error has occured while in Rx!"
						
						rxbuffer = ""
					
if fd: fd.close()
sys.exit(1)


"""
VERSION LOG                                    
===========                                    
$Log: client.py,v $
Revision 1.1.1.1  2009/04/15 14:14:36  dtb26
initial importation of v1 code

"""