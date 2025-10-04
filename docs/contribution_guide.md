# Ulysses Firmware Contribution Guide

A guide for contributing to the Ulysses flight controller firmware project.

## Git
Git is our version control system. Quick reference: https://rogerdudler.github.io/git-guide/

## GitHub
All repositories are hosted on GitHub for centralized source code management and collaboration.

## Issue Tracking
Use GitHub Issues with appropriate labels:
- **bug**: Something isn't working
- **enhancement**: New feature or improvement
- **documentation**: Documentation improvements
- **good first issue**: Suitable for newcomers
- **safety-critical**: Issues affecting flight safety
- **hardware**: Hardware-specific issues
- **sensor**: Sensor integration problems
- **control**: Flight control algorithm issues

When creating issues, provide:
1. Clear problem description
2. Expected vs actual behavior
3. Steps to reproduce
4. Hardware configuration (STM32 variant, sensors, etc.)
5. Firmware version/commit hash
6. Flight logs if applicable

## Development Branch Structure

This project uses a **development branch workflow** optimized for flight controller safety:

### Branch Types
- **main**: Production-ready, flight-tested code only
- **develop**: Integration branch for new features
- **feature branches**: Individual feature development
- **hotfix branches**: Critical bug fixes for production

### Branch Naming Convention
`<type>/<initials>-<feature-name>-<issue-number>`

Examples:
- `feature/js-imu-calibration-#45`
- `bugfix/mk-motor-esc-timeout-#23`
- `hotfix/critical-failsafe-#89`

### Development Workflow
1. Create feature branch from `develop`
2. Develop and test thoroughly
3. Submit PR to `develop`
4. After review/testing, merge to `develop`
5. Periodically merge `develop` to `main` after flight testing

## Commits
Follow Conventional Commits: https://gist.github.com/qoomon/5dfcdf8eec66a051ecd85625518cfd13

Format:
```
<type>(<scope>): <description>

<optional body>

<optional footer>
```

### Commit Types for Flight Controller
- **feat**: New features (sensors, control algorithms)
- **fix**: Bug fixes
- **perf**: Performance improvements
- **refactor**: Code restructuring
- **test**: Adding/updating tests
- **docs**: Documentation updates
- **config**: Configuration changes
- **safety**: Safety-related changes

### Scope Examples
- **imu**: Inertial measurement unit
- **gps**: GPS functionality
- **motors**: Motor control
- **telemetry**: Data transmission
- **pid**: PID controllers
- **sensors**: General sensor code
- **failsafe**: Safety mechanisms

Example commits:
```
feat(imu): add magnetometer calibration routine

fix(motors): resolve ESC timeout during rapid throttle changes

safety(failsafe): implement low battery auto-landing
```

## Pull Requests
### Before Creating PR
- [ ] Code compiles without warnings
- [ ] All unit tests pass
- [ ] Real hardware testing completed
- [ ] No flight-critical regressions introduced
- [ ] Documentation updated if needed

### PR Requirements
- Clear description of changes
- Test results and methodology
- Hardware compatibility notes
- Performance impact assessment
- Safety considerations

Keep PRs focused and under 500 lines when possible.

## Code Review
### Reviewers Should Check
- Code quality and style consistency
- Flight safety implications
- Real-time performance impact
- Memory usage (stack/heap)
- Interrupt safety
- Hardware compatibility

### Safety-Critical Review
Code affecting flight safety requires:
- Two reviewer approvals
- Hardware testing verification
- Documentation of safety analysis

## Testing Requirements
### Mandatory Tests
1. **Unit Tests**: All new functions
2. **Integration Tests**: Component interactions
3. **Hardware Tests**: Actual STM32 hardware
4. **Flight Tests**: For control algorithm changes

### Test Environments
- **Bench Testing**: Real hardware, controlled environment
- **Flight Testing**: Actual flight conditions

## Merging to Main
### Requirements
- All CI/CD tests pass
- Code review approved
- Hardware testing completed
- Flight testing verified (for control changes)
- Documentation updated
- Version tagged appropriately

**Never merge untested or broken code to main.**

### Post-Merge
- Delete feature branches
- Update project documentation
- Notify team of significant changes
- Schedule regression testing if needed
