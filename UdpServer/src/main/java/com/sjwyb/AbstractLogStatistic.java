package com.sjwyb;

import static java.util.Calendar.HOUR_OF_DAY;
import static java.util.Calendar.MILLISECOND;
import static java.util.Calendar.MINUTE;
import static java.util.Calendar.SECOND;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Calendar;

public abstract class AbstractLogStatistic {
	protected static final int TODAY = 1;
	protected static final int LAST_3_DAYS = 2;
	protected static final int LAST_7_DAYS = 3;
	protected static final int LAST_15_DAYS = 4;
	protected static final int LAST_30_DAYS = 5;
	
	private final long now;
	private final long today;
	private final long before3Days;
	private final long before7Days;
	private final long before15Days;
	private final long before30Days;
	private final long[] timePoints;
	
	protected final long[] counters;
	
	public AbstractLogStatistic() {
		Calendar calendar = Calendar.getInstance();

		now = calendar.getTimeInMillis();		
		calendar.set(HOUR_OF_DAY, 0);
		calendar.set(MINUTE, 0);
		calendar.set(SECOND, 0);
		calendar.set(MILLISECOND, 0);
		today = calendar.getTimeInMillis();
		before3Days = today - 2 * 24 * 3600 * 1000L;
		before7Days = today - 6 * 24 * 3600 * 1000L;
		before15Days = today - 14 * 24 * 3600 * 1000L;
		before30Days = today - 29 * 24 * 3600 * 1000L;
		
		timePoints = new long[] {
			now, today, before3Days, before7Days, before15Days, before30Days
		};
		
		counters = new long[timePoints.length];
		for (int i = 0; i < counters.length; i++) {
			counters[i] = 0L;
		}
	}

	abstract long parseTimeStamp(String line);
	
	abstract boolean incrementCounter(String line);
	
	protected void handleLogRecord() throws IOException {
		BufferedReader reader = new BufferedReader(new InputStreamReader(
				System.in));
		String line = null;
		while ((line = reader.readLine()) != null) {
			long ts = parseTimeStamp(line);
			for (int i = 0; i < timePoints.length; i++) {
				if (ts >= timePoints[i]) {
					if (incrementCounter(line)) {
						counters[i]++;
					}
					break;
				}
			}

		}

		for (int i = 0; i < counters.length - 1; i++) {
			counters[i + 1] += counters[i];
		}
	}
}
