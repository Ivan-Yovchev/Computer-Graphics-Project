(function() {
    let assert = require('assert');
    function dot(a, b) {
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    }
    function cross(a, b) {
        return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]];
    }
    function length(a) {
        return Math.sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    }
    function normalize(v) {
        let l = length(v);
        return [v[0] / l, v[1] / l, v[2] / l];
    }
    function correctFaceWrap(indices) {
        let texData = [tc[indices[0][0] - 1], tc[indices[1][0] - 1], tc[indices[2][0] - 1]];
        let x = texData.map(t => t[0])
        let minX = Math.min(x[0], x[1], x[2]);
        let maxX = Math.max(x[0], x[1], x[2]);
        indices[0][1] = indices[0][0];
        indices[1][1] = indices[1][0];
        indices[2][1] = indices[2][0];
        if(maxX - minX > 0.5) {
            texData.forEach(function(tex, idx) {
                if(tex[0] < 0.000001) {
                    let ntc = tc.length;
                    tc[ntc] = [1, tex[1]];
                    indices[idx][1] = ntc + 1;
                }
            });
        }
    }

    let fs = require('fs');
    let lines = fs.readFileSync('sphere.obj', {encoding: 'utf8'});
    lines = lines.split('\n');
    let data = {};
    data.o = lines.filter(s => s.startsWith('o '));
    data.v = lines.filter(s => s.startsWith('v '));
    data.vn = lines.filter(s => s.startsWith('vn '));
    data.vt = lines.filter(s => s.startsWith('vt '));
    data.f = lines.filter(s => s.startsWith('f '));

    data.f = data.f.map(function(line) {
        let coords = line.split(/\s+/g).slice(1);
        assert(coords.length == 3);
        let reversed = [];
        reversed[2] = coords[0];
        reversed[1] = coords[1];
        reversed[0] = coords[2];
        return 'f ' + reversed.join(' ');
    });

    let out = '';
    if(data.o.length > 0) {
        out += data.o.join('\n') + '\n';
    }
    out += data.v.join('\n') + '\n';
    out += data.vn.join('\n') + '\n';
    if(data.vt.length > 0) {
        out += data.vt.join('\n') + '\n';
    }
    out += data.f.join('\n');
    console.log(out);
})();