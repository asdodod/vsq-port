import fs from 'node:fs';
import assert from 'node:assert/strict';

const [wasmPath, referencePath] = process.argv.slice(2);
const { instance } = await WebAssembly.instantiate(fs.readFileSync(wasmPath));
const native = instance.exports;
const reference = fs.readFileSync(referencePath);
const f = Math.fround;
const texture = new Float32Array(32 * 32 * 32 * 3);
for (let i = 0; i < texture.length; ++i) {
    const byte = Math.trunc(f(f(f(reference.readDoubleLE(i * 8)) * 255) + .5));
    texture[i] = f(f(f(byte / 255) * 2) - 1);
}

// Independent eight-texel texture filter, including normalized half-texel
// addressing and repeat wrapping. Test the production sampler, not a copy.
function sample(x, y, z, channel) {
    const coordinates = [x, y, z].map(value => f(f(f(value) * 32) - .5));
    const indices = coordinates.map(Math.floor);
    const fractions = coordinates.map((value, i) => value - indices[i]);
    let sum = 0;
    for (let dz = 0; dz < 2; ++dz)
        for (let dy = 0; dy < 2; ++dy)
            for (let dx = 0; dx < 2; ++dx) {
                const ix = (indices[0] + dx) & 31, iy = (indices[1] + dy) & 31, iz = (indices[2] + dz) & 31;
                const weight = (dx ? fractions[0] : 1 - fractions[0]) *
                    (dy ? fractions[1] : 1 - fractions[1]) * (dz ? fractions[2] : 1 - fractions[2]);
                sum += texture[((iz * 32 + iy) * 32 + ix) * 3 + channel] * weight;
            }
    return sum;
}
function close(actual, expected, label) {
    assert(Math.abs(actual - expected) < .000004, `${label}: ${actual} != ${expected}`);
}
let seed = 12345;
function random() {
    seed = (Math.imul(seed, 1664525) + 1013904223) >>> 0;
    return seed / 2 ** 32;
}
let samples = 0;
for (let z = 0; z < 32; ++z)
    for (let y = 0; y < 32; ++y)
        for (let x = 0; x < 32; ++x)
            for (let channel = 0; channel < 3; ++channel)
                close(native.NoiseChannel((x + .5) / 32, (y + .5) / 32, (z + .5) / 32, channel),
                    texture[((z * 32 + y) * 32 + x) * 3 + channel], 'seeded texture center');
for (let i = 0; i < 5000; ++i) {
    const coords = Array.from({length: 3}, () => f(random() * 20 - 10));
    for (let channel = 0; channel < 3; ++channel) {
        close(native.NoiseChannel(...coords, channel), sample(...coords, channel), `filtered sample ${i}/${channel}`);
        ++samples;
    }
}
for (const x of [-1, -.5 / 32, 0, .5 / 32, 1, 1 + .5 / 32])
    for (let channel = 0; channel < 3; ++channel)
        close(native.NoiseChannel(x, .25, -.25, channel), sample(x, .25, -.25, channel), 'repeat boundary');
for (let i = 0; i < 500; ++i) {
    const world = Array.from({length: 3}, () => f(random() * 4 - 2));
    const scale = f(1 + random() * 9), scroll = f(random()), amount = f(random() * .35);
    const coords = world.map(value => f(f(f(value * scale) * .03125) + scroll));
    for (let channel = 0; channel < 3; ++channel)
        close(native.DisplacedChannel(...world, scale, scroll, amount, channel),
            amount <= .0001 ? world[channel] : f(world[channel] + f(sample(...coords, channel) * amount)),
            `world displacement (amount ${amount})`);
}
for (const amount of [0, .00001, .0001, -.2])
    for (let channel = 0; channel < 3; ++channel)
        assert.equal(native.DisplacedChannel(1, 2, 3, 10, .2, amount, channel), channel + 1, 'disabled noise');
console.log(`Trail noise passed: 32768 seeded texels, ${samples} filtered channels, repeat boundaries, 1500 displacement channels and disabled-noise cases.`);
