// https://adventofcode.com/2025/day/6

const fs = require('fs');
const path = require('path');


function readInput() {
  let filePath = path.join(__dirname + '/input/', 'input.txt');
  let data = fs.readFileSync(filePath, 'utf-8').split('\n');
  // Remove trailing empty lines but keep the structure
  while (data.length > 0 && data[data.length - 1] === '') {
    data.pop();
  }
  return data;
}

function part1(data) {
  // The last row contains operators, other rows contain numbers
  // Problems are arranged in columns, separated by columns of all spaces

  const operatorRow = data[data.length - 1];
  const numberRows = data.slice(0, -1);

  // Find the width of the grid
  const width = Math.max(...data.map(row => row.length));

  // Identify problem boundaries by finding columns that are all spaces
  // Parse columns to identify where problems start and end
  let problems = [];
  let currentProblemStart = null;

  for (let col = 0; col <= width; col++) {
    // Check if this column is all spaces (separator) or we've reached the end
    let isAllSpaces = true;
    for (let row = 0; row < data.length; row++) {
      const char = data[row][col] || ' ';
      if (char !== ' ') {
        isAllSpaces = false;
        break;
      }
    }

    if (isAllSpaces || col === width) {
      // End of a problem
      if (currentProblemStart !== null) {
        problems.push({
          start: currentProblemStart,
          end: col
        });
        currentProblemStart = null;
      }
    } else {
      // Part of a problem
      if (currentProblemStart === null) {
        currentProblemStart = col;
      }
    }
  }

  // Now process each problem
  let grandTotal = 0;

  for (const problem of problems) {
    // Extract the operator from the last row
    let operatorStr = operatorRow.slice(problem.start, problem.end).trim();
    let operator = operatorStr[0]; // Should be + or *

    // Extract numbers from each row
    let numbers = [];
    for (const row of numberRows) {
      let numStr = row.slice(problem.start, problem.end).trim();
      if (numStr !== '') {
        numbers.push(parseInt(numStr, 10));
      }
    }

    // Calculate result
    let result;
    if (operator === '+') {
      result = numbers.reduce((a, b) => a + b, 0);
    } else if (operator === '*') {
      result = numbers.reduce((a, b) => a * b, 1);
    }

    grandTotal += result;
  }

  return grandTotal;
}

function part2(data) {
  const opRow = data[data.length - 1];
  const numRows = data.slice(0, -1);
  const width = Math.max(...data.map(r => r.length));

  // Find problem boundaries (columns of all spaces)
  let problems = [],
    start = null;
  for (let c = 0; c <= width; c++) {
    const allSpace = c === width || data.every(r => (r[c] || ' ') === ' ');
    if (allSpace && start !== null) {
      problems.push([start, c]);
      start = null;
    } else if (!allSpace && start === null) start = c;
  }

  return problems.reverse().reduce((total, [s, e]) => {
    const op = opRow.slice(s, e).trim()[0];
    const nums = [];
    for (let c = e - 1; c >= s; c--) {
      const digits = numRows.map(r => r[c] || ' ').filter(ch => ch >= '0' && ch <= '9').join('');
      if (digits) nums.push(+digits);
    }
    return total + (op === '+' ? nums.reduce((a, b) => a + b, 0) : nums.reduce((a, b) => a * b, 1));
  }, 0);
}

function main() {
  let inputData = readInput();
  let resultPart1 = part1(inputData);
  console.log(`Part 1: ${resultPart1}`);
  let resultPart2 = part2(inputData);
  console.log(`Part 2: ${resultPart2}`);
}

main();