// (Some) dimensions from datasheet:
// https://www.waveshare.com/wiki/ESP32-S3-Tiny

free_cad = true;

$fs=0.03;
eps = 0.005;
gs = free_cad ? 1/2.54 : 1; // FreeCAD bug workaround

pcb_thickness = gs*1.5;
pcb_width = gs*18;
pcb_height = gs*23.5;
pcb_radius = gs*1;

pad_width = gs*1.6;
padA_pitch = gs*1.38;
padB_pitch = gs*1.59;
pad_pitch = gs*2.54;
pad9_pos = gs*1.59;
pad19_pos = gs*3.92;

via_size = gs*0.8/2;

mcu_size = gs*[7, 7, 0.9];
mcu_pos = [gs*5.8, gs*9.4, pcb_thickness];
antenna_size = gs*[5.5, 2, 1];
antenna_pos = [pcb_width/2 - antenna_size[0]/2, pcb_height - antenna_size[1] - gs*0.5, pcb_thickness];
zif_size = gs*5.7;
zif_pos = [(pcb_width-zif_size)/2, gs*3.5, pcb_thickness];
led_size = gs*[1.7, 1.3, 0.9];
led_pos = [gs*13, gs*4.4, pcb_thickness];

gold = [0.83, 0.69, 0.22];
black = [0.25, 0.25, 0.25];
blue = [0, 0, 0.4];
red = [0.5, 0, 0];
white = [0.85, 0.85, 0.85];

module rounded_pcb(width, height, radius) {
    translate([0, radius, 0]) cube([width, height - 2*radius, pcb_thickness]);
    translate([radius, 0, 0]) cube([width - 2*radius, height, pcb_thickness]);
    for (x = [0: 1: 1]) for (y = [0: 1: 1]) {
        translate([radius + x * (width - 2*radius), radius + y * (height - 2*radius), 0])
            cylinder(pcb_thickness, r=radius);
    }
}

module pad(x, y, pitch, orientation) {
    width = pad_width;
    radius = width/2;
    length = pitch + radius;
    color(gold) difference() {
        translate([x, y, 0]) rotate([0, 0, orientation]) translate([-radius, -eps, -eps]) {
            translate([radius, length-radius, 0]) cylinder(pcb_thickness + 2*eps, r=radius);
            cube([width, length-radius, pcb_thickness + 2*eps]);
        }
        pad_holes(x, y, pitch, orientation);
    }
}

module pad_holes(x, y, length, orientation) {
    translate([x, y, -0.5]) rotate([0, 0, orientation]) union() {
        cylinder(pcb_thickness + 1, r=via_size);
        translate([0, length, 0]) cylinder(pcb_thickness + 1, r=via_size);
    }
}

translate([-pcb_width/2, -pcb_height/2, 0]) union () {
    // PCB with holes
    color(blue) difference() {
        rounded_pcb(pcb_width, pcb_height, pcb_radius);
        for (i = [0: 1: 8]) pad_holes(0, pad9_pos + i*pad_pitch, padA_pitch, -90);
        for (i = [0: 1: 8]) pad_holes(pcb_width, pad9_pos + i*pad_pitch, padA_pitch, 90);
        for (i = [0: 1: 4]) pad_holes(pad19_pos + i * pad_pitch, 0, padB_pitch, 0);
    }
    
    // Pads
    for (i = [0: 1: 8]) pad(0, pad9_pos + i*pad_pitch, padA_pitch, -90);
    for (i = [0: 1: 8]) pad(pcb_width, pad9_pos + i*pad_pitch, padA_pitch, 90);
    for (i = [0: 1: 4]) pad(pad19_pos + i * pad_pitch, 0, padB_pitch, 0);

    // Components
    color(black) translate(mcu_pos) cube(mcu_size);
    color(red) translate(antenna_pos) cube(antenna_size);
    color(black) translate(zif_pos) translate([gs*0.45, gs*1.9, gs*0.75]) cube([zif_size - gs*0.9, gs*1.5, gs*0.4]);
    color(white) translate(zif_pos) cube([zif_size, gs*2.2, gs*1.1]);
    color(white) translate(led_pos) cube(led_size);
}
