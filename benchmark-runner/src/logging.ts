import * as winston from 'winston';
import { createLogger as createWinstonLogger, Logger, format } from 'winston';
const { combine, label, timestamp, printf } = format;

const myFormat = printf(({ level, message, label, timestamp }) => {
  return `${timestamp} [${label}] ${level}: ${message}`;
});

export function createLogger(module: string, level: 'info' | 'debug'): Logger {
  return createWinstonLogger({
    level: level,
    transports: [new winston.transports.Console(), new winston.transports.File({ filename: `logs/${module}.log` })],
    format: combine(timestamp(), label({ label: module }), myFormat),
  });
}
