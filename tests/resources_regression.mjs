import fs from 'node:fs';
import path from 'node:path';
import assert from 'node:assert/strict';

const [wasmFile, referenceFile, presetDirectory] = process.argv.slice(2);
const { instance } = await WebAssembly.instantiate(fs.readFileSync(wasmFile));
const native = instance.exports;
const offset = native.NoiseSamples();
const samples = new Float64Array(native.memory.buffer, offset, 32 * 32 * 32 * 3);
const reference = fs.readFileSync(referenceFile);
assert.equal(reference.length, samples.length * 8);
for (let i = 0; i < samples.length; ++i)
    assert.equal(samples[i], reference.readDoubleLE(i * 8), `System.Random sample ${i}`);
assert.equal(native.ReversedWinding(1), 0);
assert.equal(native.ReversedWinding(2), 1);
assert.equal(native.ConvertedZ(0.25, 1), 0.25);
assert.equal(native.ConvertedZ(0.25, 2), -0.25);
console.log(`NOISE_REFERENCE_OK: ${samples.length} samples match seeded .NET Random`);

if (presetDirectory) {
    const wiimote = JSON.parse(fs.readFileSync(path.join(presetDirectory, 'vain-wiimote-menu.vainsaber')));
    const flame = JSON.parse(fs.readFileSync(path.join(presetDirectory, 'vain-flame-redux.vainsaber')));
    assert.equal(wiimote.Version, 2);
    const objPart = wiimote.Parts.find(part => part.GeometryMode === 3 || part.GeometryMode === 'Obj');
    assert(objPart?.ObjBase64);
    const obj = Buffer.from(objPart.ObjBase64, 'base64').toString('utf8');
    let vertices = 0, normals = 0, faces = 0;
    for (const line of obj.split(/\r?\n/)) {
        const tokens = line.trim().split(/\s+/);
        if (tokens[0] === 'v' || tokens[0] === 'vn') {
            const z = Math.fround(Number(tokens[3]));
            assert(Number.isFinite(z));
            assert.equal(native.ConvertedZ(z, 2), -z);
            assert.equal(native.ConvertedZ(z, 1), z);
            if (tokens[0] === 'v') vertices++; else normals++;
        } else if (tokens[0] === 'f') faces++;
    }
    assert(vertices > 0 && normals > 0 && faces > 0);
    console.log(`WIIMOTE_COORDINATES_OK: ${vertices} vertices, ${normals} normals, ${faces} faces`);

    assert.equal(flame.Version, 2);
    assert.equal(flame.BladeTrails.length, 5);
    const pngSignature = Buffer.from('89504e470d0a1a0a', 'hex');
    let textures = 0, noiseTrails = 0;
    for (const trail of flame.BladeTrails) {
        if (trail.NoiseEnabled) {
            assert(trail.NoiseIntensity > 0 && trail.NoiseScale > 0 && trail.NoiseSpeed >= 0);
            noiseTrails++;
        }
        for (const key of ['ColorTextureBase64', 'GlowTextureBase64']) {
            if (!trail[key]) continue;
            const png = Buffer.from(trail[key], 'base64');
            assert(png.subarray(0, 8).equals(pngSignature));
            assert(png.readUInt32BE(16) > 0 && png.readUInt32BE(20) > 0);
            textures++;
        }
    }
    assert.equal(textures, 6);
    assert.equal(noiseTrails, 4);
    console.log(`FLAME_ASSETS_OK: ${textures} embedded PNGs, ${noiseTrails} noisy blade trails`);
}
