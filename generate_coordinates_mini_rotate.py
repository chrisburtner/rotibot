nums = [1,2,3,4,4,3,2,1,1,2,3,4]
letters = ["A", "B", "C"]
x_0 = 166500
y_0 = 0
dx_well = 16500
dy_well = 16500
dx_plate = 83500
dy_plate = 55500

with open("platecoordinates.dat", "w") as f:
    for i in range(6):
        plate_x = x_0 - (i / 2) * dx_plate
        plate_y = y_0 + (i % 2) * dy_plate
        for j in range(12):
            well_x = plate_x - (j / 4) * dx_well
            well_y = plate_y + (nums[j] - 1) * dy_well

            well = letters[j / 4]

            out = str(i + 1) + well + str(nums[j]) + ","
            out += str(well_x) + "," + str(well_y) + "\n"

            f.write(out)
