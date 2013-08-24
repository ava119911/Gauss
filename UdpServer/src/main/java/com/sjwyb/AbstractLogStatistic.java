package com.sjwyb;

import static java.util.Calendar.HOUR_OF_DAY;
import static java.util.Calendar.MILLISECOND;
import static java.util.Calendar.MINUTE;
import static java.util.Calendar.SECOND;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

public abstract class AbstractLogStatistic {
	protected static final int TODAY = 1;
	protected static final int LAST_3_DAYS = 2;
	protected static final int LAST_7_DAYS = 3;
	protected static final int LAST_15_DAYS = 4;
	protected static final int LAST_30_DAYS = 5;
	protected static final int LAST_90_DAYS = 6;
	protected static final int LAST_180_DAYS = 7;
	protected static final int LAST_360_DAYS = 8;
	
	private final SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd");
	
	private final long now;
	private final long today;
	private final long before3Days;
	private final long before7Days;
	private final long before15Days;
	private final long before30Days;
	private final long before90Days;
	private final long before180Days;
	private final long before360Days;
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
		before90Days = today - 89 * 24 * 3600 * 1000L;
		before180Days = today - 179 * 24 * 3600 * 1000L;
		before360Days = today - 359 * 24 * 3600 * 1000L;
		
		timePoints = new long[] {
			now, today, before3Days, before7Days, before15Days, 
			before30Days, before90Days, before180Days, before360Days
		};
		
		counters = new long[timePoints.length];
		for (int i = 0; i < counters.length; i++) {
			counters[i] = 0L;
		}
	}

	abstract long parseTimeStamp(String line);
	
	abstract boolean incrementCounter(String date, String line);
	
	protected void handleLogRecord() throws IOException {
		BufferedReader reader = new BufferedReader(new InputStreamReader(
				System.in));
		String line = null;
		while ((line = reader.readLine()) != null) {
			long ts = parseTimeStamp(line);
			for (int i = 0; i < timePoints.length; i++) {
				if (ts >= timePoints[i]) {
					if (incrementCounter(sdf.format(new Date(ts)), line)) {
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
