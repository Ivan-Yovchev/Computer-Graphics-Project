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
    let data = fs.readFileSync('sphere.obj', {encoding: 'utf8'});
    data = data.split('\n');
    let vertices = data.filter(s => s.startsWith('v ')).map(s => s.split(/\s+/g).slice(1).map(s => parseFloat(s)));
    let x = [1, 0, 0];
    let y = [0, 1, 0];
    let z = [0, 0, -1];
    let tc = vertices.map(function(p) {
        let p_u_projected = normalize(cross(y, cross(p, y)));
        let u_rad = Math.atan2(dot(z, p_u_projected), dot(x, p_u_projected));
        let v_rad = Math.acos(dot(y, p));
        let u = (1 + u_rad / Math.PI / 2) % 1;
        if(u < 0 && u > -0.000001) {
            u = 1.0;
        }
        let v = 1 - v_rad / Math.PI;
        if(isNaN(u) || v < 0.000001 || Math.abs(v - 1) < 0.000001) {
            u = null;
        }
        return [u, v];
    });
    let vn = data.filter(s => s.startsWith('vn '));
    let f = data.filter(s => s.startsWith('f ')).map(s => s.split(/\s+/g).slice(1));
    f = f.map(function(face) {
        let indices = face.map(s => s.split('/').map(s => s.length > 0 ? parseInt(s) : null));
        let texData = [tc[indices[0][0] - 1], tc[indices[1][0] - 1], tc[indices[2][0] - 1]];
        if(texData[0][0] == null || texData[1][0] == null || texData[2][0] == null) {
            let nullIndex = texData.map((v, i) => v[0] == null ? i : null).filter(v => v != null);
            assert(nullIndex.length == 1, 'nullIndex.length == 1');
            nullIndex = nullIndex[0];
            let pts = texData.map((v, i) => v[0] == null ? null : v).filter(v => v != null);
            assert(pts.length == 2, 'pts.length == 2');
            assert(Math.abs(pts[0][1] - pts[1][1]) < 0.000001, 'pts[0][1] == pts[1][1]');
            let corrected = false;
            indices[0][1] = indices[0][0];
            indices[1][1] = indices[1][0];
            indices[2][1] = indices[2][0];
            if(Math.abs(pts[0][0] - pts[1][0]) > 0.25) {
                texData.forEach(function(tex, idx) {
                    if(tex[0] != null && tex[0] < 0.000001) {
                        let ntc = tc.length;
                        tc[ntc] = [1, tex[1]];
                        indices[idx][1] = ntc + 1;
                        corrected = true;
                    }
                });
            }
            if(corrected) {
                texData = [tc[indices[0][1] - 1], tc[indices[1][1] - 1], tc[indices[2][1] - 1]];
                pts = texData.map((v, i) => v[0] == null ? null : v).filter(v => v != null);
            }
            let ntc = tc.length;
            tc[ntc] = [(pts[0][0] + pts[1][0]) / 2, texData[nullIndex][1]];
            indices[nullIndex][1] = ntc + 1;
        } else {
            correctFaceWrap(indices);
        }
        return indices.map(a => a.join('/')).join(' ');
    });
    var o = 'o Sphere\n';
    o += vertices.map(a => 'v ' + a.map(n => n.toFixed(6)).join(' ')).join('\n') + '\n';
    o += vn.join('\n') + '\n';
    o += tc.map(a => 'vt ' + a.map(n => n == null ? (0.5).toFixed(6) : n.toFixed(6)).join(' ')).join('\n') + '\n';
    o += f.map(s => 'f ' + s).join('\n') + '\n';
    console.log(o);
})();