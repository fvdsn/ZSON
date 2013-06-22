$(function(){
    $('#to_zson').click(function(){
        try{
            $('#error-input').css({'display':'none'});
            var obj = eval('('+$('#json-input').val()+')');
            var zson = ZSON.encode(obj);
        }catch(e){
            $('#error-input').text(e.toString());
            $('#error-input').css({'display':'block'});
            return;
        }
        
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
                });
            });
        });
    });
        
    $('#filename-input').change(function(event){
        $('#error-output').css({'display':'none'});
        var file = event.target.file || event.target.files[0];
        var reader = new FileReader();
        reader.onload = function(event){
            try{
                $('#json-output').val(JSON.stringify(ZSON.decode(event.target.result)));
            }catch(e){
                $('#error-output').text(e.toString());
                $('#error-output').css({'display':'block'});
            }
        };
        reader.readAsArrayBuffer(file);
    });
});
