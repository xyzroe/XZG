/*
The MIT License (MIT)

Copyright (c) 2013. Blake C. Miner. http://blakeminer.com

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

https://github.com/bminer/intel-hex.js
*/

//Intel Hex record types
const DATA = 0,
	END_OF_FILE = 1,
	EXT_SEGMENT_ADDR = 2,
	START_SEGMENT_ADDR = 3,
	EXT_LINEAR_ADDR = 4,
	START_LINEAR_ADDR = 5;

const EMPTY_VALUE = 0xFF;

/* intel_hex.parse(data)
	- `data` - Intel Hex file (string in ASCII format or Buffer Object)
	- `bufferSize` - the size of the Buffer containing the data (optional)
	  The data exceeding the buffer size will be discarded
	- `addressOffset` - starting address offset (optional)
	  The data before the starting address will be discarded

	returns an Object with the following properties:

	- `data` - data as a Buffer Object, **padded with 0xFF
	  where data is empty**.
	- `startSegmentAddress` - the address provided by the last
	  start segment address record; null, if not given
	- `startLinearAddress` - the address provided by the last
	  start linear address record; null, if not given

	Special thanks to: http://en.wikipedia.org/wiki/Intel_HEX
*/
exports.parse = function parseIntelHex(data, bufferSize, addressOffset) {
	if(data instanceof Buffer)
		data = data.toString("ascii");
	//Initialization
	var buf = Buffer.alloc(bufferSize || 8192),
		bufLength = 0, //Length of data in the buffer
		highAddress = 0, //upper address
		startSegmentAddress = null,
		startLinearAddress = null,
		addressOffset = addressOffset || 0,
		lineNum = 0, //Line number in the Intel Hex string
		pos = 0; //Current position in the Intel Hex string

	const SMALLEST_LINE = 11;
	while(pos + SMALLEST_LINE <= data.length)
	{
		//Parse an entire line
		if(data.charAt(pos++) != ":")
			throw new Error("Line " + (lineNum+1) +
				" does not start with a colon (:).");
		else
			lineNum++;
		//Number of bytes (hex digit pairs) in the data field
		var dataLength = parseInt(data.substr(pos, 2), 16);
		pos += 2;
		//Get 16-bit address (big-endian)
		var lowAddress = parseInt(data.substr(pos, 4), 16);
		pos += 4;
		//Record type
		var recordType = parseInt(data.substr(pos, 2), 16);
		pos += 2;
		//Data field (hex-encoded string)
		var dataField = data.substr(pos, dataLength * 2),
			dataFieldBuf = Buffer.from(dataField, "hex");
		pos += dataLength * 2;
		//Checksum
		var checksum = parseInt(data.substr(pos, 2), 16);
		pos += 2;
		//Validate checksum
		var calcChecksum = (dataLength + (lowAddress >> 8) +
			lowAddress + recordType) & 0xFF;
		for(var i = 0; i < dataLength; i++)
			calcChecksum = (calcChecksum + dataFieldBuf[i]) & 0xFF;
		calcChecksum = (0x100 - calcChecksum) & 0xFF;
		if(checksum != calcChecksum)
			throw new Error("Invalid checksum on line " + lineNum +
				": got " + checksum + ", but expected " + calcChecksum);
		//Parse the record based on its recordType
		switch(recordType)
		{
			case DATA:
				var absoluteAddress = highAddress + lowAddress - addressOffset;
				//Expand buf, if necessary
				if(absoluteAddress + dataLength >= buf.length)
				{
					var tmp = Buffer.alloc((absoluteAddress + dataLength) * 2);
					buf.copy(tmp, 0, 0, bufLength);
					buf = tmp;
				}
				//Write over skipped bytes with EMPTY_VALUE
				if(absoluteAddress > bufLength)
					buf.fill(EMPTY_VALUE, bufLength, absoluteAddress);
				//Write the dataFieldBuf to buf
				dataFieldBuf.copy(buf, absoluteAddress);
				bufLength = Math.max(bufLength, absoluteAddress + dataLength);
				// Safely abort if the buffer length is already sufficient
				if(bufLength >= bufferSize) {
					return {
						"data": buf.slice(0, bufLength),
						"startSegmentAddress": startSegmentAddress,
						"startLinearAddress": startLinearAddress
					};
				}
				break;
			case END_OF_FILE:
				if(dataLength != 0)
					throw new Error("Invalid EOF record on line " +
						lineNum + ".");
				return {
					"data": buf.slice(0, bufLength),
					"startSegmentAddress": startSegmentAddress,
					"startLinearAddress": startLinearAddress
				};
				break;
			case EXT_SEGMENT_ADDR:
				if(dataLength != 2 || lowAddress != 0)
					throw new Error("Invalid extended segment address record on line " +
						lineNum + ".");
				highAddress = parseInt(dataField, 16) << 4;
				break;
			case START_SEGMENT_ADDR:
				if(dataLength != 4 || lowAddress != 0)
					throw new Error("Invalid start segment address record on line " +
						lineNum + ".");
				startSegmentAddress = parseInt(dataField, 16);
				break;
			case EXT_LINEAR_ADDR:
				if(dataLength != 2 || lowAddress != 0)
					throw new Error("Invalid extended linear address record on line " +
						lineNum + ".");
				highAddress = parseInt(dataField, 16) << 16;
				break;
			case START_LINEAR_ADDR:
				if(dataLength != 4 || lowAddress != 0)
					throw new Error("Invalid start linear address record on line " +
						lineNum + ".");
				startLinearAddress = parseInt(dataField, 16);
				break;
			default:
				throw new Error("Invalid record type (" + recordType +
					") on line " + lineNum);
				break;
		}
		//Advance to the next line
		if(data.charAt(pos) == "\r")
			pos++;
		if(data.charAt(pos) == "\n")
			pos++;
	}
	throw new Error("Unexpected end of input: missing or invalid EOF record.");
};
