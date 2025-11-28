//  SPDX-FileCopyrightText: © 2025 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT
//
//  Comprehensive tag filtering test suite
//  Tests all possible combinations of tag filters based on the truth table
//  in docs/manual.md

#include "tst.h"

tstsuite("Comprehensive Tag Filter Tests")
{
    // ========================================================================
    // Test Cases with Different Tag Patterns
    // ========================================================================
    
    // No tags - should ALWAYS run regardless of filters
    tstcase("Untagged test") {
        tstcheck(1 == 1, "Untagged tests always run");
    }
    
    // Single positive tag
    tstcase("Test with +TAG", +TAG) {
        tstcheck(1 == 1, "Has +TAG");
    }
    
    // Single negative tag
    tstcase("Test with -TAG", -TAG) {
        tstcheck(1 == 1, "Has -TAG");
    }
    
    // Different positive tag
    tstcase("Test with +OTHER", +OTHER) {
        tstcheck(1 == 1, "Has +OTHER");
    }
    
    // Different negative tag
    tstcase("Test with -OTHER", -OTHER) {
        tstcheck(1 == 1, "Has -OTHER");
    }
    
    // Multiple positive tags (should match if any match)
    tstcase("Test with +TAG and +OTHER", +TAG, +OTHER) {
        tstcheck(1 == 1, "Has both +TAG and +OTHER");
    }
    
    // Multiple negative tags
    tstcase("Test with -TAG and -OTHER", -TAG, -OTHER) {
        tstcheck(1 == 1, "Has both -TAG and -OTHER");
    }
    
    // Mixed positive and negative tags
    tstcase("Test with +TAG and -OTHER", +TAG, -OTHER) {
        tstcheck(1 == 1, "Has +TAG and -OTHER");
    }
    
    tstcase("Test with -TAG and +OTHER", -TAG, +OTHER) {
        tstcheck(1 == 1, "Has -TAG and +OTHER");
    }
    
    // ========================================================================
    // Additional tag combinations for thorough testing
    // ========================================================================
    
    tstcase("Multiple positive same tags", +Alpha, +Beta, +Gamma) {
        tstcheck(1 == 1, "Three positive tags");
    }
    
    tstcase("Multiple negative same tags", -Alpha, -Beta, -Gamma) {
        tstcheck(1 == 1, "Three negative tags");
    }
    
    tstcase("Complex mix", +Database, -Slow, +Integration, -Manual) {
        tstcheck(1 == 1, "Complex tag combination");
    }
    
    // Edge cases for tag names
    tstcase("Tag with numbers", +Tag123, -Tag456) {
        tstcheck(1 == 1, "Tags can contain numbers");
    }
    
    tstcase("Tag with underscores", +My_Tag, -Other_Tag) {
        tstcheck(1 == 1, "Tags can contain underscores");
    }
    
    tstcase("Long tag name", +VeryLongTagNameThatIsStillValidAndShouldWork) {
        tstcheck(1 == 1, "Long tag names should work");
    }
    
    // Case sensitivity test
    tstcase("Lowercase tags", +lowercase, -UPPERCASE) {
        tstcheck(1 == 1, "Tag names are case-sensitive");
    }
    
    // ========================================================================
    // Test cases to verify left-to-right filter processing
    // ========================================================================
    
    tstcase("Override test positive", +Override) {
        tstcheck(1 == 1, "For testing filter override behavior");
    }
    
    tstcase("Override test negative", -Override) {
        tstcheck(1 == 1, "For testing filter override behavior");
    }
    
    // ========================================================================
    // Test cases for wildcard behavior
    // ========================================================================
    
    tstcase("Only positive tags", +Wild1, +Wild2) {
        tstcheck(1 == 1, "Should run with +* wildcard");
    }
    
    tstcase("Only negative tags", -Wild1, -Wild2) {
        tstcheck(1 == 1, "Should NOT run with +* wildcard");
    }
    
    tstcase("Mixed for wildcard", +Positive, -Negative) {
        tstcheck(1 == 1, "Has positive tag, should run with +*");
    }
}
