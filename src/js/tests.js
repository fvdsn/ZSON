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
    console.log('... all tests passed');
});
