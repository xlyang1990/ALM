#
#  Si_fitting2.py
#
#  This is an example to run ALM in the fitting mode.
#

from alm import ALM
import numpy as np

lavec = [[20.406, 0, 0],
         [0, 20.406, 0],
         [0, 0, 20.406]]
xcoord = [[ 0.0000000000000000, 0.0000000000000000, 0.0000000000000000],
          [ 0.0000000000000000, 0.0000000000000000, 0.5000000000000000],
          [ 0.0000000000000000, 0.2500000000000000, 0.2500000000000000],
          [ 0.0000000000000000, 0.2500000000000000, 0.7500000000000000],
          [ 0.0000000000000000, 0.5000000000000000, 0.0000000000000000],
          [ 0.0000000000000000, 0.5000000000000000, 0.5000000000000000],
          [ 0.0000000000000000, 0.7500000000000000, 0.2500000000000000],
          [ 0.0000000000000000, 0.7500000000000000, 0.7500000000000000],
          [ 0.1250000000000000, 0.1250000000000000, 0.1250000000000000],
          [ 0.1250000000000000, 0.1250000000000000, 0.6250000000000000],
          [ 0.1250000000000000, 0.3750000000000000, 0.3750000000000000],
          [ 0.1250000000000000, 0.3750000000000000, 0.8750000000000000],
          [ 0.1250000000000000, 0.6250000000000000, 0.1250000000000000],
          [ 0.1250000000000000, 0.6250000000000000, 0.6250000000000000],
          [ 0.1250000000000000, 0.8750000000000000, 0.3750000000000000],
          [ 0.1250000000000000, 0.8750000000000000, 0.8750000000000000],
          [ 0.2500000000000000, 0.0000000000000000, 0.2500000000000000],
          [ 0.2500000000000000, 0.0000000000000000, 0.7500000000000000],
          [ 0.2500000000000000, 0.2500000000000000, 0.0000000000000000],
          [ 0.2500000000000000, 0.2500000000000000, 0.5000000000000000],
          [ 0.2500000000000000, 0.5000000000000000, 0.2500000000000000],
          [ 0.2500000000000000, 0.5000000000000000, 0.7500000000000000],
          [ 0.2500000000000000, 0.7500000000000000, 0.0000000000000000],
          [ 0.2500000000000000, 0.7500000000000000, 0.5000000000000000],
          [ 0.3750000000000000, 0.1250000000000000, 0.3750000000000000],
          [ 0.3750000000000000, 0.1250000000000000, 0.8750000000000000],
          [ 0.3750000000000000, 0.3750000000000000, 0.1250000000000000],
          [ 0.3750000000000000, 0.3750000000000000, 0.6250000000000000],
          [ 0.3750000000000000, 0.6250000000000000, 0.3750000000000000],
          [ 0.3750000000000000, 0.6250000000000000, 0.8750000000000000],
          [ 0.3750000000000000, 0.8750000000000000, 0.1250000000000000],
          [ 0.3750000000000000, 0.8750000000000000, 0.6250000000000000],
          [ 0.5000000000000000, 0.0000000000000000, 0.0000000000000000],
          [ 0.5000000000000000, 0.0000000000000000, 0.5000000000000000],
          [ 0.5000000000000000, 0.2500000000000000, 0.2500000000000000],
          [ 0.5000000000000000, 0.2500000000000000, 0.7500000000000000],
          [ 0.5000000000000000, 0.5000000000000000, 0.0000000000000000],
          [ 0.5000000000000000, 0.5000000000000000, 0.5000000000000000],
          [ 0.5000000000000000, 0.7500000000000000, 0.2500000000000000],
          [ 0.5000000000000000, 0.7500000000000000, 0.7500000000000000],
          [ 0.6250000000000000, 0.1250000000000000, 0.1250000000000000],
          [ 0.6250000000000000, 0.1250000000000000, 0.6250000000000000],
          [ 0.6250000000000000, 0.3750000000000000, 0.3750000000000000],
          [ 0.6250000000000000, 0.3750000000000000, 0.8750000000000000],
          [ 0.6250000000000000, 0.6250000000000000, 0.1250000000000000],
          [ 0.6250000000000000, 0.6250000000000000, 0.6250000000000000],
          [ 0.6250000000000000, 0.8750000000000000, 0.3750000000000000],
          [ 0.6250000000000000, 0.8750000000000000, 0.8750000000000000],
          [ 0.7500000000000000, 0.0000000000000000, 0.2500000000000000],
          [ 0.7500000000000000, 0.0000000000000000, 0.7500000000000000],
          [ 0.7500000000000000, 0.2500000000000000, 0.0000000000000000],
          [ 0.7500000000000000, 0.2500000000000000, 0.5000000000000000],
          [ 0.7500000000000000, 0.5000000000000000, 0.2500000000000000],
          [ 0.7500000000000000, 0.5000000000000000, 0.7500000000000000],
          [ 0.7500000000000000, 0.7500000000000000, 0.0000000000000000],
          [ 0.7500000000000000, 0.7500000000000000, 0.5000000000000000],
          [ 0.8750000000000000, 0.1250000000000000, 0.3750000000000000],
          [ 0.8750000000000000, 0.1250000000000000, 0.8750000000000000],
          [ 0.8750000000000000, 0.3750000000000000, 0.1250000000000000],
          [ 0.8750000000000000, 0.3750000000000000, 0.6250000000000000],
          [ 0.8750000000000000, 0.6250000000000000, 0.3750000000000000],
          [ 0.8750000000000000, 0.6250000000000000, 0.8750000000000000],
          [ 0.8750000000000000, 0.8750000000000000, 0.1250000000000000],
          [ 0.8750000000000000, 0.8750000000000000, 0.6250000000000000]]

kd = [14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
      14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
      14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
      14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14];

force = np.loadtxt("reference/force.dat").reshape((-1, 64, 3))[:22]
disp = np.loadtxt("reference/disp.dat").reshape((-1, 64, 3))[:22]

# alm.alm_new() and alm.alm_delete() are done by 'with' statement
with ALM() as alm:
    alm.set_cell(lavec, xcoord, kd)
    alm.set_norder(2)
    alm.set_cutoff_radii([-1, 7.3])
    alm.set_displacement_and_force(disp, force)
    alm.run_fitting()
    fc_values, elem_indices = alm.get_fc(2)
    c = "xyz"
    for (fc, elem) in zip(fc_values, elem_indices):
        v1 = elem[0] // 3
        c1 = elem[0] % 3
        v2 = elem[1] // 3
        c2 = elem[1] % 3
        v3 = elem[2] // 3
        c3 = elem[2] % 3
        print("%f %d%s %d%s %d%s" % ((fc,
                                      v1 + 1, c[c1],
                                      v2 + 1, c[c2],
                                      v3 + 1, c[c3])))
