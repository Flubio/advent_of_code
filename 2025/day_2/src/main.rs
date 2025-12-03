fn main() {
    let result_part1 = sol_part1();
    println!("Part 1: {}", result_part1);
    let result_part2 = sol_part2();
    println!("Part 2: {}", result_part2);
}


fn sol_part1() -> i64 {
    let input:String = read_input();
    let id_ranges = get_ranges(input);
    let mut total_invalid_ids: i64 = 0;
    for (lower_bound, upper_bound) in id_ranges {
        // (The ID ranges are wrapped here for legibility; in your input, they appear on a single long line.)

        // The ranges are separated by commas (,); each range gives its first ID and last ID separated by a dash (-).

        // Since the young Elf was just doing silly patterns, you can find the invalid IDs by looking for any ID which is made only of some sequence of digits repeated twice. So, 55 (5 twice), 6464 (64 twice), and 123123 (123 twice) would all be invalid IDs.

        // None of the numbers have leading zeroes; 0101 isn't an ID at all. (101 is a valid ID that you would ignore.)

        // Your job is to find all of the invalid IDs that appear in the given ranges. In the above example:

        // 11-22 has two invalid IDs, 11 and 22.
        // 95-115 has one invalid ID, 99.
        // 998-1012 has one invalid ID, 1010.
        // 1188511880-1188511890 has one invalid ID, 1188511885.
        // 222220-222224 has one invalid ID, 222222.
        // 1698522-1698528 contains no invalid IDs.
        // 446443-446449 has one invalid ID, 446446.
        // 38593856-38593862 has one invalid ID, 38593859.
        // The rest of the ranges contain no invalid IDs.
        // Adding up all the invalid IDs in this example produces 1227775554.
        for id in lower_bound..=upper_bound {
            let id_str = id.to_string();
            let len = id_str.len();
            if len % 2 == 0 {
                let (first_half, second_half) = id_str.split_at(len / 2);
                if first_half == second_half {
                    total_invalid_ids += id;
                }
            }
        }
    }

    total_invalid_ids
}


fn sol_part2() -> i64 {
    let input:String = read_input();
    // Now, an ID is invalid if it is made only of some sequence of digits repeated at least twice. So, 12341234 (1234 two times), 123123123 (123 three times), 1212121212 (12 five times), and 1111111 (1 seven times) are all invalid IDs.

    // From the same example as before:

    // 11-22 still has two invalid IDs, 11 and 22.
    // 95-115 now has two invalid IDs, 99 and 111.
    // 998-1012 now has two invalid IDs, 999 and 1010.
    // 1188511880-1188511890 still has one invalid ID, 1188511885.
    // 222220-222224 still has one invalid ID, 222222.
    // 1698522-1698528 still contains no invalid IDs.
    // 446443-446449 still has one invalid ID, 446446.
    // 38593856-38593862 still has one invalid ID, 38593859.
    // 565653-565659 now has one invalid ID, 565656.
    // 824824821-824824827 now has one invalid ID, 824824824.
    // 2121212118-2121212124 now has one invalid ID, 2121212121.
    // Adding up all the invalid IDs in this example produces 4174379265.
    let ranges = get_ranges(input);
    let mut total_invalid_ids: i64 = 0;
    for (lower_bound, upper_bound) in ranges {
        for id in lower_bound..=upper_bound {
            let id_str = id.to_string();
            let len = id_str.len();
            for sub_len in 1..=(len / 2) {
                if len % sub_len == 0 {
                    let sub_str = &id_str[0..sub_len];
                    let repeated_str = sub_str.repeat(len / sub_len);
                    if repeated_str == id_str {
                        total_invalid_ids += id;
                        break;
                    }
                }
            }
        }
    }
    total_invalid_ids

}

fn get_ranges(input: String) -> Vec<(i64, i64)> {
    let id_ranges = input.split(",");
    let mut ranges: Vec<(i64, i64)> = Vec::new();
    for range in id_ranges {
        let bounds:Vec<&str> = range.split("-").collect();
        let lower_bound: i64 = bounds[0].parse().unwrap();
        let upper_bound: i64 = bounds[1].parse().unwrap();
        ranges.push((lower_bound, upper_bound));
    }
    ranges
}

fn read_input() -> String {
    std::fs::read_to_string("input/input.txt").expect("Failed to read input file")
}