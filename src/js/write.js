$(function(){
    $('#to_zson').click(function(){
        var zson = ZSON.encode(eval($('#json-input').val()));
        
        window.webkitRequestFileSystem(window.TEMPORARY, zson.byteLength || 1024*1024, function(fs) {
            fs.root.getFile($('#filename-output').val(), {create: true}, function(fileEntry) {
                fileEntry.createWriter(function(fileWriter) {
                    var arr = new Uint8Array(zson);
        
                    var blob = new Blob([arr]);
        
                    fileWriter.addEventListener("writeend", function() {
                        // navigate to file, will download
                        location.href = fileEntry.toURL();
                    }, false);
        
                    fileWriter.write(blob);
                }, function() {});
            }, function() {});
        }, function() {});
    });
        
    $('#filename-input').change(function(event){
        var file = event.target.file || event.target.files[0];
        var reader = new FileReader();
        reader.onload = function(event){
            $('#json-output').val(JSON.stringify(ZSON.decode(event.target.result)));
        };
        reader.readAsArrayBuffer(file);
    });
});
