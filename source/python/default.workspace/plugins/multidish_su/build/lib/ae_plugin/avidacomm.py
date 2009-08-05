#!C:\python\python
"""
// ----------------------------------------------------------------------------
// NAME    : avidacomm.py [IMPLEMENTATION]                                         
// PURPOSE : Implementation of the Avida-Ed communications protocol.  This lib
//           includes all symbolics, definitions and routines required to 
//           create and transmit requests and responses.  The handling of the 
//           requests and responses is left to application implementation.
//
//           Minimal sanity checks are employed within the transmission
//           routines, but these should not be relied on; the receiving
//           application is responsible for the integrity of the packet prior
//           to making use of the data sent.
//
//           *Note:  The version of Python employed saddled us with a number
//           of limitations that were circumvented by employing rather bizarre
//           code conventions (such as wrapping all hex numerals in chr()).
//           By far the greatest oddity is Python's inability to byte stream or
//           essentially to transmit anything other than a Python string object.
//           As a result, dealing with numerics is a bit tedious and has had an
//           impact on the design of the protocol (such as integers being
//           transmitted as 14-byte string values).                                      
//                                                                              
// FILE     : $Source: /cvsroot/fit140/avidacomm.py,v $                          
// REVISION : $Revision: 1.1.1.1 $                                                  
// AUTHOR   : $Author: dtb26 $                                                  
// DATE     : $Date: 2009/04/15 14:14:36 $                                      
// TAG NAME : $Name:  $                                                         
// LOCKER   : $Locker:  $                                                       
// STATE    : $State: Exp $                                                     
// ----------------------------------------------------------------------------
""" 

# -- SYMBOLICS
import socktools


# protocol-defined transmission control characters
NUL = chr(0x00)
ETX = chr(0x03)
EOT = chr(0x04)
ACK = chr(0x06)
NAK = chr(0x07)
ETB = chr(0x17)


# protocol commands
CMD_JOIN_REQ   = chr(0x10)
CMD_SUBMIT_REQ = chr(0x11)
CMD_UPDATE_REQ = chr(0x12)
CMD_SEND_REQ   = chr(0x13)
CMD_REMOVE_REQ = chr(0x14)
# @WRE: Add two command types
CMD_GENERIC_MSG  = chr(0x15)
CMD_GENERIC_RSP = chr(0x16)
# back to protocol demand defs
CMD_JOIN_RSP   = chr(0x64)
CMD_SUBMIT_RSP = chr(0x65)
CMD_UPDATE_RSP = chr(0x66)
CMD_SEND_RSP   = chr(0x67)
CMD_REMOVE_RSP = chr(0x68)

# protocol type flags
TYPE_SUBMIT_BY_FILE = chr(0x01)
TYPE_UPDATE_FULL    = chr(0x01)
TYPE_UPDATE_INCR    = chr(0x02)
TYPE_FILE_ASCII     = chr(0x01)
TYPE_FILE_BINARY    = chr(0x02)

# protocol errors
ERR_NO_ERROR        = 0x00
ERR_BAD_TX          = 0x01
ERR_BAD_CLIID       = 0x02
ERR_BAD_DISHPOS     = 0x03
ERR_BAD_DISH_ID     = 0x04
ERR_EMPTY_DISH_POS  = 0x05
ERR_DISHPOS_FULL    = 0x06
ERR_BAD_CMD         = 0x07
ERR_BAD_RSP         = 0x08
ERR_SFTWR_SYNC      = 0x09
ERR_VER_SYNC        = 0x0A
ERR_INC_REQ         = 0x0B
ERR_LOST_ID         = 0x0C
ERR_BAD_TRANS_TYPE  = 0x0D
ERR_LOST_FILE_SZ    = 0x0E
ERR_FULL_COMM       = 0x0F
ERR_RUN_ONGOING     = 0x10
ERR_RUN_IDLE        = 0x11
ERR_UPDATE_UNAVAIL  = 0x12
ERR_UPDATE_UNWARR   = 0x13
ERR_BAD_UPDATE_TYPE = 0x14
ERR_UNEXPCT_EOT     = 0x15
ERR_LOST_EOT        = 0x16
ERR_DISC_IO         = 0x17
ERR_NO_CMD          = 0x18
ERR_CLOSED          = 0x19

# misc protocol symbolics
AVIDA_SOFTWARE_ID   = "avidaedntwrk"
AVIDA_SOFTWARE_VER  = "100"
AVIDA_CURRENT_ERROR = ERR_NO_ERROR
AVIDA_DISH_POS_MIN  = 1
AVIDA_DISH_POS_MAX  = 25

# error strings 
ERR_STRING = []
ERR_STRING.append( "ERR_NO_ERROR(" + str(ERR_NO_ERROR) + "): The operation has completed successfully!" )
ERR_STRING.append( "ERR_BAD_TX(" + str(ERR_BAD_TX) + "): An error has occured while transmitting data, send() returned before completeing transmission!" )
ERR_STRING.append( "ERR_BAD_CLIID(" + str(ERR_BAD_CLIID) + "): An invalid or malformed client id was submitted.  Restrict IDs to alphanumeric and white space characters." )
ERR_STRING.append( "ERR_BAD_DISHPOS(" + str(ERR_BAD_DISHPOS) + "): The requested dish position is out of bounds (min:" + str(AVIDA_DISH_POS_MIN) + ",max:" + str(AVIDA_DISH_POS_MAX) + ")." )
ERR_STRING.append( "ERR_BAD_DISH_ID(" + str(ERR_BAD_DISH_ID) + "): There is currently no dish in the community associated to the supplied Client ID." )
ERR_STRING.append( "ERR_EMPTY_DISH_POS(" + str(ERR_EMPTY_DISH_POS) + "): The requested dish position is empty." )
ERR_STRING.append( "ERR_DISHPOS_FULL(" + str(ERR_DISHPOS_FULL) + "): The requested dish position is currently occupied." )
ERR_STRING.append( "ERR_BAD_CMD(" + str(ERR_BAD_CMD) + "): The supplied command is unknown." )
ERR_STRING.append( "ERR_BAD_RSP(" + str(ERR_BAD_RSP) + "): The supplied response flag is unknown, responses must be in the form of ACK or NAK." )
ERR_STRING.append( "ERR_SFTWR_SYNC(" + str(ERR_SFTWR_SYNC) + "): The software attempting to connect is not a recognized Avida-Ed client." )
ERR_STRING.append( "ERR_VER_SYNC(" + str(ERR_VER_SYNC) + "): The Avida-Ed client attempting to connect is running an incompatible version." )
ERR_STRING.append( "ERR_INC_REQ(" + str(ERR_INC_REQ) + "): The submitted request was incomplete and could not be processed." )
ERR_STRING.append( "ERR_LOST_ID(" + str(ERR_LOST_ID) + "): The required Client Id string was missing" )
ERR_STRING.append( "ERR_BAD_TRANS_TYPE(" + str(ERR_BAD_TRANS_TYPE) + "): The required transmission type flag was unknown." )
ERR_STRING.append( "ERR_LOST_FILE_SZ(" + str(ERR_LOST_FILE_SZ) + "): The required file size parameter was zero or missing." )
ERR_STRING.append( "ERR_FULL_COMM(" + str(ERR_FULL_COMM) + "): The community is full and cannot accept additional dishes." )
ERR_STRING.append( "ERR_RUN_ONGOING(" + str(ERR_RUN_ONGOING) + "): The community has already begun its run and cannot accept any additional dishes." )
ERR_STRING.append( "ERR_RUN_IDLE(" + str(ERR_RUN_IDLE) + "): The community has not yet begun its run." )
ERR_STRING.append( "ERR_UPDATE_UNAVAIL(" + str(ERR_UPDATE_UNAVAIL) + "): There is no update available at this time." )
ERR_STRING.append( "ERR_UPDATE_UNWARR(" + str(ERR_UPDATE_UNWARR) + "): The request for an update is unwarranted at this time." )
ERR_STRING.append( "ERR_BAD_UPDATE_TYPE(" + str(ERR_BAD_UPDATE_TYPE) + "): The requested update form is invalid, only full updates are currently supported" )
ERR_STRING.append( "ERR_UNEXPCT_EOT(" + str(ERR_UNEXPCT_EOT) + "): The transmitted file terminated prior to achieving the advertised size." )
ERR_STRING.append( "ERR_LOST_EOT(" + str(ERR_LOST_EOT) + "): The transmitted file appears to be missing an end-of-transmission marker." )
ERR_STRING.append( "ERR_DISC_IO(" + str(ERR_DISC_IO) + "): A system error has occured while reading from or writing to disk." )
ERR_STRING.append( "ERR_NO_CMD(" + str(ERR_NO_CMD) + "): Data received is missing the required command parameter." )
ERR_STRING.append( "ERR_CLOSED(" + str(ERR_CLOSED) + "): The community is closed and not accepting connections at this time." )


# -- DEFINITIONS
def getlastError(): return self.last_error	
def errorToString( error_code ): return ERR_STRING[ int(error_code) ]	
def setLastError( error_code ): AVIDA_CURRENT_ERROR = error_code

"""
// -------------------------------------------------------------------
// NAME    : getCommand()
// PURPOSE : Parses out and returns the command value in an avida-ed
//           application-layer packet.  The entire header is not parsed,
//           only the command value.
// ARGS    : packet - an avida application layer packet
// RETURNS : int - the command value
// NOTES   : none
// --------------------------------------------------------------------
""" 	
def getCommand( packet ): return packet[0]


"""
// -------------------------------------------------------------------
// NAME    : getPayload()
// PURPOSE : Parses and returns the payload (including trailer) out of
//           a supplied packet.
// ARGS    : packet - an avida application layer packet
// RETURNS : string - payload and trailer of packet
// NOTES   : This should be updated to exclude the trailer.  Original 
//           design did not include a trailer; having to employ one
//           after the fact would have resulted in a lot of code changes
//           in the test environment.
// --------------------------------------------------------------------
""" 	
def getPayload( packet ): return packet[3:len(packet)]


"""
// -------------------------------------------------------------------
// NAME    : JoinRequest()
// PURPOSE : A client would like to participate in the community.  This 
//           request is sent prior to submission or updating.  Software 
//           name and version is compared for compatibility purposes.
// ARGS    : fd - file descriptor or socket
// RETURNS : sets and returns errorno
// NOTES   : C -> S
// --------------------------------------------------------------------
""" 	
def JoinRequest( fd ):
	status = ERR_NO_ERROR
	header = str(CMD_JOIN_REQ) + NUL + NUL
	payload = AVIDA_SOFTWARE_ID + ETX + str(AVIDA_SOFTWARE_VER).zfill(6)
	trailer = ETB
	packet = header + payload + trailer
	packet_sz = len(packet)
	
	transmitted = socktools.toSocket( fd, packet_sz, packet )	
	if transmitted < packet_sz: status = ERR_BAD_TX
	
	setLastError( status )
	return status
	
#eof JoinRequest()


"""
// -------------------------------------------------------------------
// NAME    : SubmitRequest()
// PURPOSE : A client has defined a new dish and would like to have it 
//           added to the community.  The submission request is made to 
//           ensure that there is space and that the desired position 
//           is available.
// ARGS    : fd - file descriptor or socket
//           whoiam - Id string
//           pos - position in the community display
// RETURNS : int - error no
// NOTES   : C -> S
// --------------------------------------------------------------------
""" 	
def SubmitRequest( fd, whoiam, pos, type = TYPE_SUBMIT_BY_FILE ):
	status = ERR_NO_ERROR
	header = CMD_SUBMIT_REQ + NUL + NUL
	trailer = ETB
	
	# verifiy sanity
	if pos < AVIDA_DISH_POS_MIN or pos > AVIDA_DISH_POS_MAX : status = ERR_BAD_DISHPOS
	
	if status == ERR_NO_ERROR:
		payload = type + str(pos).zfill(2) + whoiam + ETX
		packet = header + payload + trailer
		packet_sz = len(packet)
		
		transmitted = socktools.toSocket( fd, packet_sz, packet )
		if transmitted < packet_sz: status = ERR_BAD_TX
		
	setLastError( status )
	return status
	
#eof SubmitRequest()
		
		
"""
// -------------------------------------------------------------------
// NAME    : UpdateRequest()
// PURPOSE : The client would like to be updated as to the state of 
//           the community.  This request is sent to ask for a current 
//           snapshot.  Note that v1 of the protocol only supports full 
//           updates.  Thus this routine only allows for the appropriate 
//           type flag and the incremental 'last update' field is left 
//           NUL.
// ARGS    : fd - file descriptor or socket
//           type - integer value indicating full or incremental update
// RETURNS : int - error no
// NOTES   : C -> S
// --------------------------------------------------------------------
""" 			
def UpdateRequest( fd, type = TYPE_UPDATE_FULL ):
	status = ERR_NO_ERROR
	header = CMD_UPDATE_REQ + NUL + NUL
	trailer = ETB
	
	# sanity chck, v1 supports only full updates
	if type != TYPE_UPDATE_FULL: status = ERR_BAD_UPDATE_TYPE
	
	if status == ERR_NO_ERROR:
		payload = type + NUL + ETX
		packet = header + payload + trailer
		packet_sz = len(packet)
		
		transmitted = socktools.toSocket( fd, packet_sz, packet )
		if transmitted < packet_sz: status = ERR_BAD_TX
		
	setLastError( status )
	return status
	
#eof UpdateRequest()
		
		
"""
// -------------------------------------------------------------------
// NAME    : SendRequest()
// PURPOSE : One side of a connection wishes to transmit a file to the
//           other.  This is accomplished via three independant pushes
//           up the pipe:  the header and identifying info is sent,
//           followed by the file, and finally the end-of-file marker
//           and trailer go.  The scheme is simplistic and is not overly
//           robust.  If file sizes tend to exceed one megabyte I would
//           consider converting this to a chunking algorithm.
// ARGS    : fd - the socket
//           filename - string name of the file to be sent
//           filesz - int size in bytes of the file to transmit
//           thefile - pointer/handle to a file object opened for read
//           type - flag indicating ascii or binary
// RETURNS : int - avida protocol defined error code
// NOTES   : C -> S || S-> C
// --------------------------------------------------------------------
""" 	
def SendRequest( fd, filename, filesz, thefile, type = TYPE_FILE_ASCII ):
	status = ERR_NO_ERROR
	header = CMD_SEND_REQ + NUL + NUL
	trailer = ETB
	
	if status == ERR_NO_ERROR:
		packet_pre_file = header + type + filename + ETX + str(filesz).zfill(14)
		packet_post_file = trailer
		pre_sz = len(packet_pre_file)
		post_sz = len(packet_post_file)
		transmitted = socktools.toSocket( fd, pre_sz, packet_pre_file )
		if transmitted != pre_sz: 
			status = ERR_BAD_TX
		else:
			transmitted = socktools.toSocket( fd, filesz, thefile.read() + EOT )
			if transmitted < filesz:
				status = ERR_BAD_TX
			else:
				transmitted = socktools.toSocket( fd, post_sz, packet_post_file )
				if transmitted != post_sz: status = ERR_BAD_TX
	
	setLastError( status )
	return status
		
#eof sendRequest()

"""
// -------------------------------------------------------------------
// NAME    : RemoveRequest()
// PURPOSE : Sends a request to remove a dish from the community.  This
//           request must be made prior to the community run, though
//           details of that limitation are left to the application
//           implementation.
// ARGS    : fd - the socket
//           whoiam - client-id string
//           pos - int position of the dish in the community matrix
// RETURNS : int - avida protocol defined error code
// NOTES   : C -> S
// --------------------------------------------------------------------
""" 	
def RemoveRequest( fd, whoiam, pos ):
	status = ERR_NO_ERROR
	header = CMD_REMOVE_REQ + NUL + NUL
	trailer = ETB
	
	# verifiy sanity
	if not whoiam.isalnum() and not (whoiam.isalnum() and whoiam.isspace()): status = ERR_BAD_CLIENT_ID
	if pos < AVIDA_DISH_POS_MIN or pos > AVIDA_DISH_POS_MAX : status = ERR_BAD_DISHPOS
	
	if status == ERR_NO_ERROR:
		payload = str(pos).zfill(2) + whoiam + ETX
		packet = header + payload + trailer
		packet_sz = len(packet)
		
		transmitted = socktools.toSocket( fd, packet_sz, packet )
		if transmitted < packet_sz: status = ERR_BAD_TX
		
	setLastError( status )
	return status
	
#eof removeRequest()
	

"""
// -------------------------------------------------------------------
// NAME    : IssueResponse()
// PURPOSE : A wrapper for issuing all responses through.
// ARGS    : fd - the socket
//           cmd - response command
//           result - ACK or NAK
//           reason - int error code (0 being no error)
//           rxbytes - not required but allows for confirming packet size recv'd
// RETURNS : int - avida protocol defined error code
// NOTES   : S -> C || C -> S
// --------------------------------------------------------------------
""" 	
def IssueResponse( fd, cmd, result, reason, rxbytes = NUL ):
	status = ERR_NO_ERROR
	header = cmd + NUL + NUL
	trailer = ETB
	
	# verifiy sanity
	if result != NAK and result != ACK: status = ERR_BAD_RSP
	
	if status == ERR_NO_ERROR:
		payload = result + str(rxbytes).zfill(14)
		packet = header + payload + trailer
		packet_sz = len(packet)
		
		transmitted = socktools.toSocket( fd, packet_sz, packet )
		if transmitted < packet_sz: status = ERR_BAD_TX
		
	setLastError( status )
	return status
	
#eof IssueResponse()


"""
VERSION LOG                                    
===========                                    
$Log: avidacomm.py,v $
Revision 1.1.1.1  2009/04/15 14:14:36  dtb26
initial importation of v1 code

"""
