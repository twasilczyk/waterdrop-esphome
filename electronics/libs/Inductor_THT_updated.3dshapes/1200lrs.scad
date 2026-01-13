$fs = 0.05;
$fa = 2;

clr = [0.4, 0.4, 0.4];

translate([2, -2.5, 0]) {

difference() {
    color(clr) cylinder(8.5, 10.5/2, 10.5/2);
    color([0.3, 0.3, 0.3]) translate([0, 0, 8])
        cylinder(1, 9/2, 9/2);
}
color(clr) translate([0, 0, 8])
    cylinder(0.5, 8/2, 8/2);

color([0.9, 0.9, 0.9]) {

translate([-2, -2.5, -3.5])
    cylinder(3.5, 0.8/2, 0.8/2);
translate([2, -2.5, -3.5])
    cylinder(3.5, 0.8/2, 0.8/2);
translate([-2, 2.5, -3.5])
    cylinder(3.5, 0.8/2, 0.8/2);
translate([2, 2.5, -3.5])
    cylinder(3.5, 0.8/2, 0.8/2);

}

}
