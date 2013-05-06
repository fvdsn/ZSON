$(function(){
    var exports = {};

    var TRUE    = 1;
    var FALSE   = 2;
    var NULL    = 3;
    var INT8    = 4;
    var INT16   = 5;
    var INT32   = 6;
    var UINT8   = 7;
    var UINT16  = 8;
    var UINT32  = 9;
    var FLOAT32 = 10;
    var FLOAT64 = 11;
    var STRING_UTF8   = 12; 
    var ARRAY         = 13;
    var OBJECT        = 14;
    var ARRAY_INT8    = 15;
    var ARRAY_INT16   = 16;
    var ARRAY_INT32   = 17;
    var ARRAY_UINT8   = 18;
    var ARRAY_UINT16  = 19;
    var ARRAY_UINT32  = 20;
    var ARRAY_FLOAT32 = 21;
    var ARRAY_FLOAT64 = 22;

    function Buffer(buffer,bigendian){
        this.buffer = buffer || (new Uint8Array([0])).buffer ;
        this.view   = new DataView(this.buffer);
        this.bigendian = bigendian || false;
        this.size = this.buffer.byteLength; 
    }
    exports.Buffer = Buffer;

    Buffer.prototype.resize = function(size){
        var src = new Uint8Array(this.buffer);
        var dst = new Uint8Array(size);
        var len = Math.min(this.buffer.byteLength,size);

        for(var i = 0; i < len; i++){
            dst[i] = src[i];
        }

        this.buffer = dst.buffer;
        this.view = new DataView(this.buffer);
        this.size = this.buffer.byteLength;
    };

    Buffer.prototype.checkSize = function(size){
        if(size > this.size){
            this.resize(Math.max(size,Math.ceil(this.size * 1.5)));
        }
    };

    Buffer.prototype.getInt8    = function(offset){ return this.view.getInt8(offset,this.bigendian)};
    Buffer.prototype.getInt16   = function(offset){ return this.view.getInt16(offset,this.bigendian)};
    Buffer.prototype.getInt32   = function(offset){ return this.view.getInt32(offset,this.bigendian)};
    Buffer.prototype.getUint8   = function(offset){ return this.view.getUint8(offset,this.bigendian)};
    Buffer.prototype.getUint16  = function(offset){ return this.view.getUint16(offset,this.bigendian)};
    Buffer.prototype.getUint32  = function(offset){ return this.view.getUint32(offset,this.bigendian)};
    Buffer.prototype.getFloat32 = function(offset){ return this.view.getFloat32(offset,this.bigendian)};
    Buffer.prototype.getFloat64 = function(offset){ return this.view.getFloat64(offset,this.bigendian)};
        
    Buffer.prototype.setInt8    = function(offset,value){
        this.checkSize(offset+1);
        this.view.setInt8(offset,value,this.bigendian);
    };
    Buffer.prototype.setInt16   = function(offset,value){
        this.checkSize(offset+2);
        this.view.setInt16(offset,value,this.bigendian);
    };
    Buffer.prototype.setInt32   = function(offset,value){
        this.checkSize(offset+4);
        this.view.setInt32(offset,value,this.bigendian);
    };
    Buffer.prototype.setUint8   = function(offset,value){
        this.checkSize(offset+1);
        this.view.setUint8(offset,value,this.bigendian);
    };
    Buffer.prototype.setUint16  = function(offset,value){
        this.checkSize(offset+2);
        this.view.setUint16(offset,value,this.bigendian);
    };
    Buffer.prototype.setUint32  = function(offset,value){
        this.checkSize(offset+4);
        this.view.setUint32(offset,value,this.bigendian);
    };
    Buffer.prototype.setFloat32 = function(offset,value){
        this.checkSize(offset+4);
        this.view.setFloat32(offset,value,this.bigendian);
    };
    Buffer.prototype.setFloat64 = function(offset,value){
        this.checkSize(offset+8);
        this.view.setFloat64(offset,value,this.bigendian);
    };

    function buffer_log(buffer){
        var a = new Uint8Array(buffer);
        var str = '';
        for(var i = 0; i < a.length; i++){
            str += a[i];
            str += ' ';
        }
        console.log(str);
    }
    exports.buffer_log = buffer_log;

    Buffer.prototype.log = function(){ buffer_log(this.buffer); }

    function padding(offset,align){
        var rst = offset % align;
        if(rst === 0){
            return rst;
        }else{
            return align - rst;
        }
    }
    exports.padding = padding;

    function header_size(bits64){
        return bits64 ? 9 : 5;
    }
    function get_size(buffer,offset,bits64){
        if(bits64){
            return buffer.getFloat64(offset+1);
        }else{
            return buffer.getUint32(offset+1);
        }
    }
    function set_size(buffer,offset,size,bits64){
        if(bits64){
            buffer.setFloat64(offset+1,size);
        }else{
            buffer.setUint32(offset+1,size);
        }
    }

    function _encode(dst,offset,val,bits64){
        var size = 1;
        if(val === true){
            dst.setUint8(offset,TRUE);
        }else if(val === false){
            dst.setUint8(offset,FALSE);
        }else if(val === null){
            dst.setUint8(offset,NULL);
        }else if(typeof val === 'number'){
            var fp = Math.round(val) !== val;
            if(!fp){
                if(val >= 0){
                    if(val < 256){
                        dst.setUint8(offset,UINT8);
                        dst.setUint8(offset+1,val);
                        size = 2;
                    }else if(val < 65536){
                        dst.setUint8(offset,UINT16);
                        dst.setUint16(offset+1,val);
                        size = 3;
                    }else if(val < 4294967296){
                        dst.setUint8(offset,UINT32);
                        dst.setUint32(offset+1,val);
                        size = 5;
                    }else{
                        fp = true;
                    }
                }else{
                    if(val > -128){
                        dst.setUint8(offset,INT8);
                        dst.setInt8(offset+1,val);
                        size = 2;
                    }else if(val > -32768){
                        dst.setUint8(offset,INT16);
                        dst.setInt16(offset+1,val);
                        size = 3;
                    }else if(val > -2147483648){
                        dst.setUint8(offset,INT32);
                        dst.setInt32(offset+1,val);
                        size = 5;
                    }else{
                        fp = true;
                    }
                }
            }
            if(fp){
                dst.setInt8(offset,FLOAT64);
                dst.setFloat64(offset+1,val);
                size = 9;
            }
        }else if(typeof val === 'string'){
            dst.setUint8(offset,STRING_UTF8);
            size = header_size(bits64);
            var utf8str = val; //unescape(encodeURIComponent(val));
            var utf8 = false;
            for(var i = 0, len = utf8str.length; i < len; i++){
                var c = utf8str.charCodeAt(i);
                if(!utf8 && (c < 32 || c > 127)){
                    utf8str = unescape(encodeURIComponent(val));
                    size = header_size(bits64);
                    i = 0;
                    utf8 = true;
                }else{
                    dst.setUint8(offset + size,c);
                    size += 1;
                }
            }
            dst.setUint8(offset+size,0);    // null string terminator
            size+= 1;
            set_size(dst,offset,size,bits64);
        }else if(val instanceof Array){
            dst.setUint8(offset,ARRAY);
            size = header_size(bits64);
            for(var i = 0, len = val.length; i < len; i++){
                size += _encode(dst, offset + size, val[i],bits64);
            }
            set_size(dst,offset,size,bits64);
        }else if(val instanceof Int8Array){
            dst.setUint8(offset,ARRAY_INT8);
            size = header_size(bits64);
            for(var i = 0, len = val.length; i < len; i++){
                dst.setInt8(offset+size,val[i]);
                size += 1;
            }
            set_size(dst,offset,size,bits64);
        }else if(val instanceof Int16Array){
            dst.setUint8(offset,ARRAY_INT16);
            size = header_size(bits64);
            size += padding(offset+size,2);
            for(var i = 0, len = val.length; i < len; i++){
                dst.setInt16(offset+size,val[i]);
                size += 2;
            }
            set_size(dst,offset,size,bits64);
        }else if(val instanceof Int32Array){
            dst.setUint8(offset,ARRAY_INT32);
            size = header_size(bits64);
            size += padding(offset+size,4);
            for(var i = 0, len = val.length; i < len; i++){
                dst.setInt32(offset+size,val[i]);
                size += 4;
            }
            set_size(dst,offset,size,bits64);
        }else if(val instanceof Uint8Array){
            dst.setUint8(offset,ARRAY_UINT8);
            size = header_size(bits64);
            for(var i = 0, len = val.length; i < len; i++){
                dst.setUint8(offset+size,val[i]);
                size += 1;
            }
            set_size(dst,offset,size,bits64);
        }else if(val instanceof Uint16Array){
            dst.setUint8(offset,ARRAY_UINT16);
            size = header_size(bits64);
            size += padding(offset+size,2);
            for(var i = 0, len = val.length; i < len; i++){
                dst.setUint16(offset+size,val[i]);
                size += 2;
            }
            set_size(dst,offset,size,bits64);
        }else if(val instanceof Uint32Array){
            dst.setUint8(offset,ARRAY_UINT32);
            size = header_size(bits64);
            size += padding(offset+size,4);
            for(var i = 0, len = val.length; i < len; i++){
                dst.setUint32(offset+size,val[i]);
                size += 4;
            }
            set_size(dst,offset,size,bits64);
        }else if(val instanceof Float32Array){
            dst.setUint8(offset,ARRAY_FLOAT32);
            size = header_size(bits64);
            size += padding(offset+size,4);
            for(var i = 0, len = val.length; i < len; i++){
                dst.setFloat32(offset+size,val[i]);
                size += 4;
            }
            dst.setUint32(offset+1,size);
        }else if(val instanceof Float64Array){
            dst.setUint8(offset,ARRAY_FLOAT64);
            size = header_size(bits64);
            size += padding(offset+size,8);
            for(var i = 0, len = val.length; i < len; i++){
                dst.setFloat64(offset+size,val[i]);
                size += 8;
            }
            set_size(dst,offset,size,bits64);
        }else{
            var obj = val;
            dst.setUint8(offset,OBJECT);
            size = header_size(bits64);
            for(key in obj){
                if(obj.hasOwnProperty(key)){
                    size += _encode(dst, offset + size, key);
                    size += _encode(dst, offset + size, obj[key]);
                }
            }
            set_size(dst,offset,size,bits64);
        }
        if(!bits64 && size > 4294967296){
            throw new Error('ZSON: Too much data to encode: '+size);
        }
        return size;
    }

    function encode(obj){
        var buf = new Buffer();
        var size = _encode(buf,0,obj,false);
        buf.resize(size);
        return buf.buffer;
    }
    exports.ZSON = {};
    exports.ZSON.encode = encode;

    function _decode(src,offset,bits64){
        var header = src.getUint8(offset);
        var ret = {data: undefined, size: 1};

        if(header === NULL){
            ret.data = null;
        }else if(header === TRUE){
            ret.data = true;
        }else if(header === FALSE){
            ret.data = false;
        }else if(header === UINT8){
            ret.data = src.getUint8(offset+1);
            ret.size = 2;
        }else if(header === UINT16){
            ret.data = src.getUint16(offset+1);
            ret.size = 3;
        }else if(header === UINT32){
            ret.data = src.getUint32(offset+1);
            ret.size = 5;
        }else if(header === INT8){
            ret.data = src.getInt8(offset+1);
            ret.size = 2;
        }else if(header === INT16){
            ret.data = src.getInt16(offset+1);
            ret.size = 3;
        }else if(header === INT32){
            ret.data = src.getInt32(offset+1);
            ret.size = 5;
        }else if(header === FLOAT32){
            ret.data = src.getFloat32(offset+1);
            ret.size = 5;
        }else if(header === FLOAT64){
            ret.data = src.getFloat64(offset+1);
            ret.size = 9;
        }else if(header === ARRAY){
            ret.data = [];
            var len = get_size(src,offset,bits64);
            var size = header_size(bits64);
            while(size < len){
                el = _decode(src,offset+size,bits64);
                ret.data.push(el.data);
                size += el.size;
            }
            ret.size = len;
        }else if(header === ARRAY_INT8){
            var len = get_size(src,offset,bits64);
            var hsize = header_size(bits64);
            ret.data = new Int8Array(src.buffer,offset+hsize,len-hsize);
            ret.size = len;
        }else if(header === ARRAY_INT16){
            var len = get_size(src,offset,bits64);
            var end = offset + len;
            var start = offset + header_size(bits64);
                start += padding(start,2);
            ret.data = new Int16Array(src.buffer,start,(end-start)/2);
            ret.size = len;
        }else if(header === ARRAY_INT32){
            var len = get_size(src,offset,bits64);
            var end = offset + len;
            var start = offset + header_size(bits64);
                start += padding(start,4);
            ret.data = new Int16Array(src.buffer,start,(end-start)/2);
            console.log(offset,start,end);
            ret.data = new Int32Array(src.buffer,start,(end-start)/4);
            ret.size = len;
        }else if(header === ARRAY_UINT8){
            var len = get_size(src,offset,bits64);
            var hsize = header_size(bits64);
            ret.data = new Uint8Array(src.buffer,offset+hsize,len-hsize);
            ret.size = len;
        }else if(header === ARRAY_UINT16){
            var len = get_size(src,offset,bits64);
            var end = offset + len;
            var start = offset + header_size(bits64);
                start += padding(start,2);
            ret.data = new Uint16Array(src.buffer,start,(end-start)/2);
            ret.size = len;
        }else if(header === ARRAY_UINT32){
            var len = get_size(src,offset,bits64);
            var end = offset + len;
            var start = offset + header_size(bits64);
                start += padding(start,4);
            ret.data = new Uint32Array(src.buffer,start,(end-start)/4);
            ret.size = len;
        }else if(header === ARRAY_FLOAT32){
            var len = get_size(src,offset,bits64);
            var end = offset + len;
            var start = offset + header_size(bits64);
                start += padding(start,4);
            ret.data = new Float32Array(src.buffer,start,(end-start)/4);
            ret.size = len;
        }else if(header === ARRAY_FLOAT64){
            var len = get_size(src,offset,bits64);
            var end = offset + len;
            var start = offset + header_size(bits64);
                start += padding(start,8);
            ret.data = new Float64Array(src.buffer,start,(end-start)/8);
            ret.size = len;
        }else if(header === OBJECT){
            ret.data = {};
            var len = get_size(src,offset,bits64);
            var size = header_size(bits64);
            while(size < len){
                key = _decode(src,offset+size,bits64);
                if(typeof key.data !== 'string'){
                    throw new Error('ZSON: Object key is not a string');
                }
                size += key.size;
                val = _decode(src,offset+size,bits64);
                ret.data[key.data] = val.data;
                size+= val.size;
            }
            ret.size = size;
        }else if(header === STRING_UTF8){
            var len = get_size(src,offset,bits64);
            var size = header_size(bits64);
            var utf8str = '';
            var utf8 = false;
            while(size < len - 1){
                var c = src.getUint8(offset+size);
                utf8str += String.fromCharCode(c);
                if(c < 32 || c >= 127){
                    utf8 = true;
                }
                size += 1;
            }
            ret.data = utf8 ? decodeURIComponent(escape(utf8str)) : utf8str;
            ret.size = len;
        }else{
            throw new Error('ZSON: Unkown entity type: '+header);
        }
        return ret;
    }

    exports.ZSON.decode = function(obj){
        var b = new Buffer(obj);
        return _decode(b,0,false).data;
    };

    for(field in exports){
        window[field] = exports[field];
    }
});

