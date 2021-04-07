import { existsSync } from 'fs';
import { promisify as p } from 'util';
import { mkdirs, readdir, lstatSync } from 'fs-extra';
const dataToLatex = require('data-to-latex');

export async function createDirectoryIfMissing(file: string) {
  if (!existsSync(file)) {
    await p(mkdirs)(file);
  }
}

export async function isEmptyDirectory(file: string): Promise<boolean> {
  try {
    const files = await readdir(file);
    return files.length === 0;
  } catch (e) {
    // non-existing => empty
    return true;
  }
}

export function isDirectory(path: string): boolean {
  return lstatSync(path).isDirectory();
}

function listSum(lst: number[]) {
  return lst.reduce(function (a, b) {
    return a + b;
  });
}

// https://www.npmjs.com/package/confidence95
export function conf(numbers: number[]) {
  const sum = listSum(numbers);
  const mean = sum / numbers.length;
  const sqerrs = numbers.map(function (n) {
    return (n - mean) * (n - mean);
  });
  const std = Math.sqrt(listSum(sqerrs) / numbers.length);
  const interval = (1.959964 * std) / Math.sqrt(numbers.length);
  return {
    mean: mean,
    std: std,
    interval: interval,
  };
}

export function constructionLatexTable(table: string[][]): string {
  let latexTable: string[] = [];

  const tabularOptions = {
    vLines: new Array(table[0].length + 1).fill(true), // set vertical lines to get a fully closed tabular
    hLines: new Array(table.length + 1).fill(true), // set horizontal lines to close it horizontally
  };

  table.forEach((row) => (latexTable = latexTable.concat(row)));
  return dataToLatex
    .formattedTabular(latexTable, table[0].length, tabularOptions)
    .toString()
    .replace(/%/g, '\\%')
    .replace(/#/g, '\\#');
}
