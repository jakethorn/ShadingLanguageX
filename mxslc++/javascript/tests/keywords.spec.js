import { test, expect } from '@playwright/test';
import { loadMxslc } from './testHelpers.js';

test.describe('MaterialX keyword extraction', () =>
{
    let mx;

    test.beforeAll(async () =>
    {
        mx = await loadMxslc();
    });

    test('returns all MaterialX nodedef category names from the stdlib', () =>
    {
        // Emscripten's register_vector returns a StringVector wrapper (not a
        // native Array). Read it via its size() / get(i) methods.
        const names = mx.getMtlxDefinitionNames();

        expect(names).toBeTruthy();

        const count = names.size();
        expect(count).toBeGreaterThan(0);

        // Print the count so it is visible in the test output.
        console.log(`- Found ${count} MaterialX nodedef categories`);

        // Sanity-check the list is unique (no duplicates, no empty strings).
        const present = new Set();
        for (let i = 0; i < count; i++)
        {
            const s = names.get(i);
            expect(s).toBeTruthy();
            present.add(s);
        }
        expect(present.size).toBe(count);

        // Sanity-check a few well-known categories are present.
        expect(present.has('add')).toBe(true);
        expect(present.has('multiply')).toBe(true);
        expect(present.has('standard_surface')).toBe(true);
    });
});
