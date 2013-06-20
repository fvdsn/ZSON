$(function(){
    window.time  = function(fun,iter){
        var startTime = (new Date()).getTime();
        iter = iter || 1;
        for(var i = 0; i < iter; i++){
            fun();
        }
        return (new Date()).getTime() - startTime;
    };
    console.log('starting unit tests ...');
    console.log('1) testing basic types');
    var vals = [null,true,false,
        0,1,2,3,4,5,10,100,127,128,129,255,256,257,1000,10000,
        65355,65356,65357,100000,1000000,-1,-2,-3,-4,-5,-10,-100,
        -127,-128,-128,-255,-256,-257,-1000,-10000,-100000,-1000000,
        1.0,3.14,Math.random(),Math.random(),Math.random(),-43131412.54,
        '','a','b','$','é','ç','@!#$%&*()_+=´";:;?/<>,.~`','abcAS-e fg hijk',
        'éçúíŕǵḱ'];
    for(var i = 0; i < vals.length; i++){
        console.assert(ZSON.decode(ZSON.encode(vals[i])) === vals[i],'roundtrip test: '+vals[i]);
    }
    console.log('2) testing typed arrays');
    var typedarrays = [
        new Int8Array([0,1,2,127,-1,-2,-128,42]),
        new Int8Array([]),
        new Int16Array([0,1,2,127,-1,-2,-128,42,255,256,-32768,32767]),
        new Int16Array([]),
        new Int32Array([0,1,2,127,-1,-2,-128,42,256,256,-32768,32767,65535,65536,-2147483648,2147483647]),
        new Int32Array([]),
        new Uint8Array([0,1,2,128,254,255]),
        new Uint8Array([]),
        new Uint16Array([0,1,2,128,254,255,65534,65535]),
        new Uint16Array([]),
        new Uint32Array([0,1,2,128,254,255,65534,65535,65536,4294967294,4294967295]),
        new Uint32Array([]),
        new Float32Array([0,1.0,-1.0,Math.PI,Math.random(),Math.random()*1000,Math.random()*10000,1/0,-1/0,0/0]),
        new Float64Array([0,1.0,-1.0,Math.PI,Math.random(),Math.random()*1000,Math.random()*10000,1/0,-1/0,0/0])
    ];
    window.typedarrays = typedarrays;
    function array_equals(a1,a2){
        if(a1.byteLength === a2.byteLength && a1.length === a2.length){
            for(var i = 0; i < a1.length; i++){
                if(!(a1[i] === a2[i] || (isNaN(a1[i]) && isNaN(a2[i])))){
                    console.log(a1[i],a2[i]);
                    return false;
                }
            }
            return true;
        }
        return false;
    }
    for(var i = 0; i < typedarrays.length; i++){
        var array = typedarrays[i];
        console.assert(array_equals(array,ZSON.decode(ZSON.encode(array))),'array test:['+i+']'+array.toString());
    }
    console.log('... finished all tests');
});
